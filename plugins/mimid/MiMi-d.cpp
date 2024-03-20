/*
	==============================================================================
	This file is part of the MiMi-d synthesizer.

	Copyright 2024 Ricard Wanderlof

	This file may be licensed under the terms of of the
	GNU General Public License Version 2 (the ``GPL'').

	Software distributed under the License is distributed
	on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
	express or implied. See the GPL for the specific language
	governing rights and limitations.

	You should have received a copy of the GPL along with this
	program. If not, go to http://www.gnu.org/licenses/gpl.html
	or write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
	==============================================================================
 */

#include "DistrhoPlugin.hpp"

// Stuff needed to bring in juce_Random.h
#define JUCE_API
#define JUCE_LEAK_DETECTOR(FOO) DISTRHO_LEAK_DETECTOR(FOO)
#include "lib/juce_MathsFunctions.h" // Need jmin,jmax,jlimit,int64
#include "lib/juce_Random.h"
using namespace juce;

#include "Engine/SynthEngine.h"
#include "Engine/Params.h"

START_NAMESPACE_DISTRHO

typedef void (SynthEngine::*SetFuncType)(float);
typedef void (*ScalePointType)(ParameterEnumerationValues &enumValues);

// Plugin class parameters are #params, #programs, #states .

// Set up enumValue member of Parameter struct from min and max value
// Used in lambda call, hence put outside class
static void setParameterEnumerationValues(ParameterEnumerationValues &enumValues, int min, int max)
{
	int nvalues = max + 1 - min;
	if (nvalues <= 1)
		return; // Leave &enumValues in its default state
	float increment = 1.0f / (nvalues - 1);
	float value = 0;
	ParameterEnumerationValue *ev =
		new ParameterEnumerationValue[nvalues];

	enumValues.values = ev;
	enumValues.count = nvalues;
	enumValues.deleteLater = true;
	enumValues.restrictedMode = true;

	for (int dispVal = min; dispVal <= max; dispVal++) {
		ev->label = String(dispVal);
		if (dispVal == max)
			value = 1.0f; // fixate last value
		ev++->value = value;
		value += increment;
	}
}

// Set up enumValue member of Parameter struct from varargs list of strings
// Used in lambda call, hence put outside class
static void setParameterEnumerationValues(ParameterEnumerationValues &enumValues, ...)
{
	va_list args;

	// Count number of values = number of varargs
	// Since there is no built in sentinel in varargs, we trust
	// our caller to have a NULL pointer at the end of the
	// argument list.
	size_t argcount = 0;
	va_start(args, enumValues);
	while (va_arg(args, char *))
		argcount++;
	va_end(args);

	// Get out now if we only have one value in our list.
	// It wouldn't make sense, and causes division by zero
	// later on.
	// We don't touch &enumValues, so it will remain in its
	// default state.
	if (argcount == 1)
		return;

	float increment = 1.0 / (argcount - 1);

	ParameterEnumerationValue *ev =
		new ParameterEnumerationValue[argcount];

	enumValues.values = ev;
	enumValues.count = argcount;
	enumValues.deleteLater = true;
	enumValues.restrictedMode = true;

	float value = 0;

	// Go through argument list again, generating strings
	// for the individual values.
	va_start(args, enumValues);
	for (size_t argno = 0; argno < argcount; argno++) {
		ev->label = va_arg(args, char *);
		ev++->value = value;
		value += increment;
	}
	va_end(args);
}

class MiMid : public Plugin {
private:
	Params parameters;
	SetFuncType setfuncs[PARAM_COUNT];
	ScalePointType scalepoints[SP_COUNT];
	SynthEngine synth;

protected:
public:
	MiMid() : Plugin(PARAM_COUNT, 0, 0)
	{
		synth.setSampleRate(getSampleRate());

		// Set up setfuncs array

#define SEFUNC(FUNCNAME) &SynthEngine::FUNCNAME

#define PARAMPOINTS(SPID, ...)
#define PARAMRANGE(SPID, MIN, MAX)
#define PARAMGROUP(PGID, NAME, SYMBOL)
#define PARAM(PARAMNO, PG, SP, NAME, SYMBOL, MIN, MAX, DEFAULT, SETFUNC) \
		setfuncs[PARAMNO] = SEFUNC(SETFUNC);
#include "Engine/ParamDefs.h"

		// Set up scalepoints array.
		// scalepoints is an array of function pointers to lambda
		// functions, one per scale point set, which are used
		// when inializing the Scale Points for relevant parameters.
		// Each lambda contains values from the corresponding
		// macro call, calling setParameterEnumerationValues()
		// with the correspnding values.

// The NULL is important as it works as a sentinel for the list of char *'
#define PARAMPOINTS(SPID, ...) \
	scalepoints[SPID] = [](ParameterEnumerationValues &enumValues) \
	{ \
		setParameterEnumerationValues(enumValues, __VA_ARGS__, NULL); \
	};
#define PARAMRANGE(SPID, MIN, MAX) \
	scalepoints[SPID] = [](ParameterEnumerationValues &enumValues) \
	{ \
		setParameterEnumerationValues(enumValues, MIN, MAX); \
	};
#define PARAMGROUP(PGID, NAME, SYMBOL)
#define PARAM(PARAMNO, PG, SP, NAME, SYMBOL, MIN, MAX, DEFAULT, SETFUNC)
#include "Engine/ParamDefs.h"

		initAllParams();
	}

protected:
	const char *getLabel() const override { return "MiMi-d"; }
	const char *getDescription() const override {
	    return "MiMi-d synthesizer";
	}
	const char *getMaker() const override { return "Pollux"; }
	const char *getLicense() const override { return "GPL2"; }
	uint32_t getVersion() const override { return d_version(1,0,0); }
	int64_t getUniqueId() const override {
	    return d_cconst('M','i','M','d');
	}

	// Doesn't seem to be necessary but just in case ...
	void initAudioPort(bool input, uint32_t index, AudioPort &port) override
	{
		port.groupId = kPortGroupStereo;

		Plugin::initAudioPort(input, index, port);
	}

	void initPortGroup(uint32_t groupId, PortGroup &portGroup) override
	{
		switch(groupId) {

#define PARAMPOINTS(SPID, ...)
#define PARAMRANGE(SPID, MIN, MAX)
#define PARAM(PARAMNO, PG, SP, NAME, SYMBOL, MIN, MAX, DEFAULT, SETFUNC)
#define PARAMGROUP(PGID, NAME, SYMBOL) \
		case PGID: \
			portGroup.name = NAME; \
			portGroup.symbol = SYMBOL; \
			break;
#include "Engine/ParamDefs.h"

		default:
			break;
		}
	}

	void initParameter(uint32_t paramno, Parameter &parameter) override
	{
		switch(paramno) {
#define PARAMPOINTS(SPID, ...)
#define PARAMRANGE(SPID, MIN, MAX)
#define PARAMGROUP(PGID, NAME, SYMBOL)
#define PARAM(PARAMNO, PG, SP, NAME, SYMBOL, MIN, MAX, DEFAULT, SETFUNC) \
		case PARAMNO: \
			parameter.name = NAME; \
			parameter.symbol = SYMBOL; \
			parameter.groupId = PG; \
			scalepoints[SP](parameter.enumValues); \
			parameter.ranges.def = DEFAULT; \
			parameter.ranges.min = MIN; \
			parameter.ranges.max = MAX; \
			break;
#include "Engine/ParamDefs.h"

		default:
			break;
		}
	}

	float getParameterValue(uint32_t paramno) const override {
		if (paramno < PARAM_COUNT)
			return parameters.values[paramno];
		else
			return 0.0;
	}

	void setParameterValue(uint32_t paramno, float value) override {
		if (paramno < PARAM_COUNT) {
			parameters.values[paramno] = value;
			if (setfuncs[paramno])
				(synth.*setfuncs[paramno])(value);
		}
	}

private:
	void initAllParams()
	{
		for (int i = 0 ; i < PARAM_COUNT; i++)
	                setParameterValue(i, parameters.values[i]);
	}

	inline void processMidiEvent(const MidiEvent *midiEvent)
	{
		(void) midiEvent;
		if (midiEvent->size > 3) // sysex?
			return;
		uint8_t status = midiEvent->data[0];
		uint8_t data1 = midiEvent->data[1];
		uint8_t data2 = midiEvent->data[2];
#define note data1
#define vel data2
#define cc data1
#define ccval data2
#define atval data1

#define MIDI_NOTE_ON 144
#define MIDI_NOTE_OFF 128
#define MIDI_CC 176
#define MIDI_BEND 224
#define MIDI_AT 208

		switch (status & 0xf0)
		{
		case MIDI_NOTE_ON:
			if (vel) {
				synth.procNoteOn(note, vel * (1/127.0));
				break;
			}
			[[fallthrough]]; // C++17 rules!
		case MIDI_NOTE_OFF:
			synth.procNoteOff(note);
			break;
		case MIDI_BEND:
			synth.procPitchWheel(((int)data2 * 128 + data1 - 8192) * (1/8192.0));
			break;
		case MIDI_CC:
			switch (cc)
			{
				case 1: // mod wheel
					synth.procModWheel(ccval * (1/127.0));
					break;
				case 64: // sustain pedal
					if (ccval)
						synth.sustainOn();
					else
						synth.sustainOff();
					break;
				case 120: // all sound off
					synth.sustainOff();
					synth.allNotesOff();
					break;
				case 123: // all notes off
					synth.allNotesOff();
					break;
				default:
					break;
			}
			break;
		case MIDI_AT:
			synth.procAfterTouch(atval * (1/127.0));
			break;
		default:
			break;
		}
#undef note
#undef vel
#undef cc
#undef ccval
#undef atval
	}

	inline void processMidiPerSample(const MidiEvent *midiEvents, uint32_t &midiEventIndex, const uint32_t midiEventCount, const uint32_t &samplePos)
	{
		while (midiEventIndex < midiEventCount && midiEvents[midiEventIndex].frame <= samplePos) {
			processMidiEvent(&midiEvents[midiEventIndex]);
			midiEventIndex++;
		}
	}

protected:
	void run(const float **inputs,
		 float **outputs,
		 uint32_t frames,
		 const MidiEvent *midiEvents,
		 uint32_t midiEventCount) override
	{
		float *outL = outputs[0];
		float *outR = outputs[1];
		(void) inputs; // synth has no audio inputs
		uint32_t samplePos = 0;
		uint32_t midiEventIndex = 0;
#if 0 // TODO: Fix playhead managment = LFO tempo sync
		if (getPlayHead() != 0 && getPlayHead()->getCurrentPosition (pos))
			synth.setPlayHead(pos.bpm,pos.ppqPosition);
#endif

		while (samplePos < frames)
		{
			processMidiPerSample(midiEvents, midiEventIndex, midiEventCount, samplePos);

			synth.processSample(outL + samplePos, outR + samplePos);

			samplePos++;
		}
	}

	void sampleRateChanged(double newSampleRate) override
	{
		synth.setSampleRate(newSampleRate);
	}

	DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MiMid);
};

Plugin *createPlugin() { return new MiMid(); }

END_NAMESPACE_DISTRHO
