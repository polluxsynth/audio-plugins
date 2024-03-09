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

// Plugin class parameters are #params, #programs, #states .

class MiMid : public Plugin {
private:
	Params parameters;
	SetFuncType setfuncs[PARAM_COUNT];
	SynthEngine synth;

public:
	MiMid() : Plugin(PARAM_COUNT, 0, 0)
	{
		synth.setSampleRate(44100);

		// Set up setfuncs array

#define SEFUNC(FUNCNAME) &SynthEngine::FUNCNAME
#define PARAM(PARAMNO, NAME, SYMBOL, MIN, MAX, DEFAULT, SETFUNC) \
		setfuncs[PARAMNO] = SEFUNC(SETFUNC);
#define PARAM_NULL(PARAMNO, NAME, SYMBOL) \
		setfuncs[PARAMNO] = NULL;
#include "Engine/ParamDefs.h"
#undef PARAM
#undef PARAM_NULL

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

	void initParameter(uint32_t paramno, Parameter &parameter) override
	{
		switch(paramno) {

#define PARAM(PARAMNO, NAME, SYMBOL, MIN, MAX, DEFAULT, SETFUNC) \
		case PARAMNO: \
			parameter.name = NAME; \
			parameter.symbol = SYMBOL; \
			parameter.ranges.def = DEFAULT; \
			parameter.ranges.min = MIN; \
			parameter.ranges.max = MAX; \
			break;
#define PARAM_NULL(PARAMNO, NAME, SYMBOL) \
		case PARAMNO: \
			parameter.name = NAME; \
			parameter.symbol = SYMBOL; \
			parameter.ranges.def = 0.0 ; \
			parameter.ranges.min = 0.0 ; \
			parameter.ranges.max = 1.0 ; \
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
