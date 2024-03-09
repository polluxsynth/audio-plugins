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

protected:
	void run(const float **inputs,
		 float **outputs,
		 uint32_t frames,
		 const MidiEvent *midiEvents,
		 uint32_t midiEventCount) override
	{
		// Just a stub for now
		(void) inputs;
		(void) outputs;
		(void) frames;
		(void) midiEvents;
		(void) midiEventCount;
	}

	DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MiMid);
};

Plugin *createPlugin() { return new MiMid(); }

END_NAMESPACE_DISTRHO
