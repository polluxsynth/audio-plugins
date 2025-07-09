/*
	==============================================================================
	This file is part of the MiMi-d synthesizer.

	Copyright 2024-2025 Ricard Wanderlof

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

#include "Engine/SynthEngine.h"
#include "Engine/Params.h"

START_NAMESPACE_DISTRHO

#define HINT_SHIFT 16 // hints are shifted up 16 bits
#define SP_MASK 0xffff // scale point count mask

typedef void (SynthEngine::*SetFuncType)(float);
typedef uint32_t (*ScalePointType)(ParameterEnumerationValues &enumValues);

// Set up enumValue member of Parameter struct from varargs list of strings
// Used in lambda call, hence put outside class
// Returns maximum value of values in enumValues, so 1 for a range of 0-1
static uint32_t setParameterEnumerationValues(ParameterEnumerationValues &enumValues, ...)
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
	// later on; we can use it as a null operation however (SP_NONE).
	// We don't touch &enumValues, so it will remain in its
	// default state.
	if (argcount == 1)
		return 0;

	ParameterEnumerationValue *ev =
		new ParameterEnumerationValue[argcount];

	enumValues.values = ev;
	enumValues.count = argcount;
	enumValues.deleteLater = true;
	enumValues.restrictedMode = true;

	int value = 0;

	// Go through argument list again, generating strings
	// for the individual values.
	va_start(args, enumValues);
	for (size_t argno = 0; argno < argcount; argno++) {
		ev->label = va_arg(args, char *);
		ev++->value = value;
		value++;
	}
	va_end(args);

	uint32_t retval = argcount - 1;
        // If there are two values, hint that the parameter is boolean
	if (argcount == 2)
		retval |= kParameterIsBoolean << HINT_SHIFT;
	return retval;
}

// Plugin class parameters are #params, #programs, #states .

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
// Parameter is enumerated, with a list of strings for the values
#define PARAMPOINTS(SPID, ...) \
	scalepoints[SPID] = [](ParameterEnumerationValues &enumValues) \
	{ \
		 return setParameterEnumerationValues(enumValues, __VA_ARGS__, NULL); \
	};

// Parameter has specific hints
#define PARAMHINTS(SPID, HINTS) \
	scalepoints[SPID] = [](ParameterEnumerationValues &enumValues) \
	{ \
		(void) enumValues; \
		return (HINTS) << HINT_SHIFT; \
	};

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
	uint32_t getVersion() const override { return d_version(2,1,1); }
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
		uint32_t sp_status, SP_MAX;
		switch(paramno) {
		// The low word of SP_MAX is used as a MAX value for
		// PARAMPOINTS parameters, in order to automatically set
		// the maximum value for the enum depending on the number
		// of values.
		// Alternatively, the high word of SP_MAX can be used to set
		// parameter hints; see PARAMHINTS.
#define PARAM(PARAMNO, PG, SP, NAME, SYMBOL, MIN, MAX, DEFAULT, SETFUNC) \
		case PARAMNO: \
			parameter.name = NAME; \
			parameter.symbol = SYMBOL; \
			parameter.groupId = PG; \
			sp_status = scalepoints[SP](parameter.enumValues); \
			SP_MAX = sp_status & SP_MASK; \
			parameter.ranges.def = DEFAULT; \
			parameter.ranges.min = MIN; \
			parameter.ranges.max = MAX; \
			parameter.hints |= sp_status >> HINT_SHIFT; \
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
#define MIDI_POLY_AT 160

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
		case MIDI_POLY_AT:
			synth.procAfterTouch(note, ccval * (1/127.0));
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
	void run(const float ** /* inputs */,
		 float **outputs,
		 uint32_t frames,
		 const MidiEvent *midiEvents,
		 uint32_t midiEventCount) override
	{
		float *outL = outputs[0];
		float *outR = outputs[1];
		uint32_t samplePos = 0;
		uint32_t midiEventIndex = 0;
		const TimePosition& timePos(getTimePosition());

		if (timePos.bbt.valid) {
			synth.setBpm(timePos.bbt.beatsPerMinute);
			if (timePos.playing) {
			// barStartTick runs from 0 to first beat of this bar
			// beat runs from 1 to end of bar
			// tick runs from 0 to ticksPerBeat
				float totalTick = timePos.bbt.barStartTick +
						  (timePos.bbt.beat - 1) *
						  timePos.bbt.ticksPerBeat +
						  timePos.bbt.tick;
				// Alternative calculation:
				//float totalTick = ((timePos.bbt.bar - 1) *
				//		   timePos.bbt.beatsPerBar +
				//		   (timePos.bbt.beat - 1)) *
				//		  timePos.bbt.ticksPerBeat +
				//		  timePos.bbt.tick;
				float beatPos = totalTick / timePos.bbt.ticksPerBeat;
				synth.setPlayHead(beatPos);
			}
		}

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
