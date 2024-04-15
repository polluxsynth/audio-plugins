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
	float s1;
	float spread;
    Random rg;
	float SampleRate;
	float SampleRateInv;

	float syncRate;
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
public:
	float Frequency;
	float phaseInc;
	float frequency;//frequency value without sync
	float rawFrequency;
	int waveForm;
	bool invert;
	bool unipolar;
	Lfo(enum WaveType default_wavetype = OFF)
	{
		phaseInc = 0;
		frequency=0;
		syncRate = 1;
		rawFrequency=0;
		clockSynced = false;
		keySynced = false;
		s1=0;
		Frequency=1;
		phase=0;
		spread=1;
		waveForm=0;
		invert=unipolar=false;
		sh=0;
		newCycle=false;
		rg=Random();
		wavetype=default_wavetype;
		oneShot=false;
		setSymmetry(0.5);
	}
	void setClockSync(bool enable)
	{
		clockSynced = enable;
		if (clockSynced)
			recalcRate(rawFrequency);
		else
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
	void hostSyncRetrigger(float bpm,float quaters)
	{
		if(clockSynced)
		{
			phaseInc = (bpm/60.0)*syncRate;
			phase = phaseInc*quaters;
			phase = (fmod(phase,1)*2-1);
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
		if (wavetype != OFF) {
			if (!unipolar)
				Res = Res * 2 - 1;
			if (invert)
				Res = -Res;
		}
		newCycle = false;
		return tptlpupw(s1, Res,3000,SampleRateInv);
	}
	void setSampleRate(float sr)
	{
		SampleRate=sr;
		SampleRateInv = 1 / SampleRate;
	}
	inline void update()
	{
		if (oneShot) {
			// Oneshot mode - stop when phase reaches 1
			if (phase < 1)
				phase+=((phaseInc * SampleRateInv));
			if (phase > 1)
				phase = 1;
		} else {
			// Normal LFO mode - reset phase when > 1
			phase+=((phaseInc * SampleRateInv));
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
		if(!clockSynced)
			phaseInc = frequency * spread;
	}
	void setRawFrequency(float param)//used for clock synced rate changes
	{
		rawFrequency = param;
		if(clockSynced)
		{
			recalcRate(param);
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
		const int ratesCount = 9;
		int parval = (int)(param * (ratesCount - 1) / 10);
		float rt = 1;
		switch(parval)
		{
		case 0:
			rt = 1.0 / 8;
			break;
		case 1:
			rt = 1.0 / 4;
			break;
		case 2:
			rt = 1.0 / 3;
			break;
		case 3:
			rt = 1.0 / 2;
			break;
		case 4:
			rt = 1.0;
			break;
		case 5:
			rt = 3.0 / 2;
			break;
		case 6:
			rt = 2;
			break;
		case 7:
			rt = 3;
			break;
		case 8:
			rt = 4;
			break;
		default:
			rt = 1;
			break;
		}
		syncRate = rt;
	}
};
