/*
	==============================================================================
        This file is part of the MiMi-d synthesizer,
        originally from Obxd synthesizer.

        Copyright © 2013-2014 Filatov Vadim
        Copyright 2023 Ricard Wanderlof

        Contact original author via email :
        justdat_@_e1.ru

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
#pragma once
#include "SynthEngine.h"
class Lfo
{
private:
	float phase; // 0 -> 1
	float sh; // peak +1/-1
	bool newCycle;
	float lpstate;
	float spread;
	SRandom rg;
	float SampleRate;
	float SampleRateInv;

	float syncRatio;
	bool clockSynced;
	bool oneShot;
	bool keySynced;

	float symmetry;
	float symmetryOffset;
	float symmetryInv;
	float symmetryRevInv;

	enum WaveType { OFF, TRIANGLE, PULSE, S_H } wavetype;

	struct WaveDef {
		enum WaveType wavetype;
		float symmetry;
	} WaveDef_Table[14] = {
		 { OFF, 0 }, // Off, (symmetry unused)
		 { TRIANGLE, 0 }, // Tri peak at 0%/Falling saw
		 { TRIANGLE, 0.10 }, // Tri peak at 10%
		 { TRIANGLE, 0.5 }, // Tri symmetric
		 { TRIANGLE, 0.90 }, // Tri peak at 90%
		 { TRIANGLE, 1 }, // Tri peak at 100%/Rising saw
		 { PULSE, 0.25 }, // Pulse 25% duty cycle
		 { PULSE, 0.5 }, // Square (symmetric pulse)
		 { PULSE, 0.75 }, // Pulse 75% duty cycle
		 { S_H, 0 }, // S/H (symmetry unused)
	};

	struct PolarityDef {
		float factor;
		float offset;
	} PolarityDef_Table[4] = {
		{ 2, -1 },	// Normal (bipolar) (-1..+1)
		{ -2, 1 },	// Invert (+1..-1)
		{ 1, 0 },	// Unipolar (0..+1)
		{ -1, 0 },	// Unipolar inverted (0..-1)
	};

	const float ratioTable[11] = {
		1.0 / 8,	// 0
		1.0 / 6,	// 1
		1.0 / 4,	// 2
		1.0 / 3,	// 3
		1.0 / 2,	// 4
		1.0,		// 5
		3.0 / 2,	// 6
		2,		// 7
		3,		// 8
		4,		// 9
		8,		// 10
	};

public:
	float phaseInc;
	float frequency; // frequency value without sync
	float rawFrequency;
	float bpm;
	int waveForm;
	float polarity_factor, polarity_offset;
	Lfo(enum WaveType default_wavetype = OFF)
	{
		phaseInc = 0;
		frequency = 0;
		bpm = 0;
		syncRatio = 1;
		rawFrequency = 0;
		clockSynced = false;
		keySynced = false;
		lpstate = 0;
		phase = 0;
		spread = 1;
		waveForm = 0;
		polarity_factor = 2.0;
		polarity_offset = -1.0;
		sh = 0;
		newCycle = false;
		rg = SRandom(SRandom::globalRandom().nextInt32());
		wavetype = default_wavetype;
		oneShot = false;
		setSymmetry(0.5);
	}
	void setClockSync(bool enable)
	{
		clockSynced = enable;
		if (clockSynced) {
			recalcRate(rawFrequency);
			phaseInc = (bpm / 60.0) * syncRatio;
		} else
			phaseInc = frequency * spread;
	}
	void setKeySync(bool enable)
	{
		keySynced = enable;
		// When turning key sync off, reset phase, so that all
		// voices run at the same phase.
		if (!keySynced)
			phase = 1;
	}
	void setSymmetryOffset()
	{
		// symmetryOffset is where the waveform starts in LFO
		// mode, relative to the oneshot/envelope mode, where
		// it starts at the lowest point. In LFO mode, the triangle
		// waveform always starts at half the amplitude (= zero
		// when the waveform is bipolar)
		symmetryOffset = oneShot ? 0 : symmetry * 0.5;
	}
	void setOneShot(bool enable)
	{
		oneShot = enable;
		setSymmetryOffset();
	}
	// Reset phase if in keySync mode
	void keyResetPhase()
	{
		if (keySynced) {
			phase = 0;
			newCycle = true;
		}
	}
	// Sync phase when voice enabled
	void phaseSync(Lfo &masterLfo)
	{
		if (!keySynced)
			// If we are voice 0, this would be an identity
			// statement, which is not really a problemm, and any
			// if statement to alleviate it will in most cases
			// just make the code path longer. In practice though,
			// this is not intended to be called for voice 0
			// anyway.
			phase = masterLfo.phase;
	}
	void setBpm(float newbpm)
	{
		bpm = newbpm;
		if(clockSynced)
			phaseInc = (bpm / 60.0) * syncRatio;
	}
	void hostSyncRetrigger(float beatpos)
	{
		if(clockSynced)
		{
			phase = syncRatio * beatpos;
			float phaseOld = phase;
			phase = fmod(phase, 1);
			// It's unlikely that the beat sync will cause the
			// phase to reset (or rather, it's much more likely
			// to happen during an ordinary sample update), but
			// if it does, trigger the S/H so we don't skip
			// a cycle.
			if (phase < phaseOld) // phase has wrapped
				newCycle = true;
		}
	}
	inline float getVal()
	{
		float Res = 0;
		float tmpPh = phase;
		switch (wavetype)
		{
			case OFF:
				 break;
			case TRIANGLE:
				tmpPh += symmetryOffset;
				tmpPh -= (tmpPh > 1); // Handle wrap
				// By using <= here, the reverse sawtooth
				// will stop at its maximum point in oneshot
				// mode, effectively turning it into an attack
				// only envelope, which is more useful than the
				// saw which drops directly to zero.
				Res = tmpPh <= symmetry ? tmpPh * symmetryInv :
							 (1 - tmpPh) * symmetryRevInv;
				break;
			case PULSE:
				Res = tmpPh < symmetry ? 1 : 0;
				break;
			case S_H:
				if (newCycle)
					sh = rg.nextFloat();
				Res = sh;
				break;
		}
		Res = Res * polarity_factor + polarity_offset;
		newCycle = false;
		return tptlpupw(lpstate, Res, 3000, SampleRateInv);
	}
	// Polarity encoding: bit 0 is 'invert' bit, bit 1 is 'unipolar' bit
	// 0: Normal: factor = 2, offset = -1 (bipolar)
	// 1: Invert: factor = -2, offset = +1
	// 2: Unipol: factor = 1, offset = 0
	// 3: UnnInv: factor = -1, offset = 0
	void setPolarity(int polarity)
	{
		struct PolarityDef &polarity_def = PolarityDef_Table[polarity];
		polarity_factor = polarity_def.factor;
		polarity_offset = polarity_def.offset;
	}
	void setSampleRate(float sr)
	{
		SampleRate=sr;
		SampleRateInv = 1 / SampleRate;
	}
	inline void update()
	{
		phase += phaseInc * SampleRateInv;
		if (oneShot) {
			// Oneshot mode - stop when phase reaches 1
			if (phase > 1)
				phase = 1;
		} else {
			// Normal LFO mode - reset phase when > 1
			if (phase > 1) {
				phase -= 1;
				newCycle = true;
			}
		}
	}
	void setSpread(float val)
	{
		spread = val;
		setFrequency(frequency);
	}
	void setFrequency(float val)
	{
		frequency = val;
		if (!clockSynced)
			phaseInc = frequency * spread;
	}
	void setRawFrequency(float param) // for clock synced rate changes
	{
		rawFrequency = param;
		if (clockSynced)
		{
			recalcRate(param);
			phaseInc = (bpm / 60.0) * syncRatio;
		}
	}
	void setSymmetry(float symm)
	{
		symmetry = symm;
		symmetryInv = symmetry > 0.0 ? 1.0 / symmetry : 0;
		symmetryRevInv = symmetry < 1.0 ? 1.0 / (1.0 - symmetry) : 0;
		setSymmetryOffset();
	}
	void setWaveForm(int select)
	{
		struct WaveDef &wavedef = WaveDef_Table[select];
		wavetype = wavedef.wavetype;
		setSymmetry(wavedef.symmetry);
	}
	void recalcRate(float param)
	{
		int intpar = roundToInt(param);

		if (intpar >= 0 && intpar <= 10)
			syncRatio = ratioTable[intpar];
	}
};
