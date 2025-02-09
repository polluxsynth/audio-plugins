/*
	==============================================================================
	This file is part of the MiMi-d synthesizer.

	Copyright © 2013-2014 Filatov Vadim
	Copyright 2023-2025 Ricard Wanderlof

	Contact author via email :
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
#include "BlepData.h"
class TrapezoidOsc
{
	bool falling;
	float prevSymm;
	float buffer1[Samples * 2];
	const int n, nmask;
	float const *blepPTR;
	float const *blampPTR;

	int bP1, bP2;
public:
	TrapezoidOsc(): n(Samples * 2), nmask(Samples * 2 - 1)
	{
		falling = false;
		prevSymm = 0;
		bP1 = bP2 = 0;
		for (int i = 0; i < n; i++)
			buffer1[i] = 0;
		blepPTR = blep;
		blampPTR = blamp;
	}
	~TrapezoidOsc()
	{
	}
	inline void setDecimation()
	{
		blepPTR = blepd2;
		blampPTR = blampd2;
	}
	inline void removeDecimation()
	{
		blepPTR = blep;
		blampPTR = blamp;
	}
	inline void processMaster(float x, float delta, float symm, float riseGrad, float fallGrad, bool waveformReset)
	{
		if (waveformReset) {
			float trans = x - delta;
			float mix = trans <= symm ? trans * riseGrad : (trans - 1.0f) * fallGrad;
			if (trans > symm)
				mixInBlampCenter(buffer1, bP1, 1.0f, (riseGrad - fallGrad) * Samples * delta);
			mixInImpulseCenter(buffer1, bP1, 1.0f, mix);
			falling = false;
			return;
		}
		float summated = delta - (symm - prevSymm);
		if (falling && x >= 1.0f) {
			x -= 1.0f;
			mixInBlampCenter(buffer1, bP1, x / delta, (fallGrad - riseGrad) * Samples * delta);
			falling = false;
		}
		if (!falling && x >= symm && x - summated < symm) {
			falling = true;
			float frac = (x - symm) / summated;
			mixInBlampCenter(buffer1, bP1, frac, (riseGrad - fallGrad) * Samples * summated);
		}
		if (falling && x >= 1.0f) {
			x -= 1.0f;
			mixInBlampCenter(buffer1, bP1, x / delta, (fallGrad - riseGrad) * Samples * delta);
			falling = false;
		}
		prevSymm = symm;
	}
	inline float getValue(float x, float symm, float riseGrad, float fallGrad)
	{
		float mix = x <= symm ? x * riseGrad : (x - 1.0f) * fallGrad;
		mix -= 0.5f; // DC level is always same regardless of symmetry
		// Instead of subtracting Samples to get to the middle of the
		// BLEP buffer, and then masking with size-1 to keep the
		// offset inside the buffer, we can just XOR the offset with
		// Samples (which corresponds to subtracting (or adding, for
		// that matter) half the buffer size and discarding the carry),
		// since the buffer is 2 * Samples long.
		buffer1[bP1 ^ Samples] += mix;
		return getNextBlep(buffer1, bP1);
	}
	inline void processSlave(float x, float delta, bool hardSyncReset, float hardSyncFrac, float symm, float riseGrad, float fallGrad)
	{
		float summated = delta - (symm - prevSymm);
		bool hspass = true;
		if (falling && x >= 1.0f) {
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				mixInBlampCenter(buffer1, bP1, x / delta, (fallGrad - riseGrad) * Samples * delta);
				falling = false;
			} else {
				x += 1.0f;
				hspass = false;
			}
		}
		if (!falling && x >= symm && x - summated < symm && hspass) {
			float frac = (x - symm) / summated;
			if (!hardSyncReset || (frac > hardSyncFrac)) { // de morgan processed equation
				falling = true;
				mixInBlampCenter(buffer1, bP1, frac, (riseGrad - fallGrad) * Samples * summated);
			}
		}
		if (falling && x >= 1.0f && hspass) {
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				mixInBlampCenter(buffer1, bP1, x / delta, (fallGrad - riseGrad) * Samples * delta);
				falling = false;
			} else {
				// if transition did not occur
				x += 1.0f;
			}
		}
		if (hardSyncReset)
		{
			// Reset the waveform, and restart it at hardSyncFrac
			float fracMaster = delta * hardSyncFrac;
			float trans = x - fracMaster;
			float mix = trans <= symm ? trans * riseGrad : (trans - 1.0f) * fallGrad;
			if (trans > symm)
				mixInBlampCenter(buffer1, bP1, hardSyncFrac, (fallGrad - riseGrad) * Samples * delta);
			mixInImpulseCenter(buffer1, bP1, hardSyncFrac, mix);
			falling = false;
			// If symmetry is very small, we might get an event
			// in the same sample period as the hard sync event
			x = fracMaster;
			if (x >= symm && x - summated < symm) {
				falling = true;
				float frac = (x - symm) / summated;
				mixInBlampCenter(buffer1, bP1, frac, (riseGrad - fallGrad) * Samples * summated);
			}

		}
		prevSymm = symm;
	}
	inline void mixInBlampCenter(float *buf, int &bpos, float offset, float scale)
	{
		int lpIn = (int)(B_OVERSAMPLING * offset);
		float frac = offset * B_OVERSAMPLING - lpIn;
		float f1 = 1.0f - frac;
		lpIn *= Blepsize;
		for (int i = 0; i < n; i++) {
			float mixvalue = blampPTR[lpIn] * f1 + blampPTR[lpIn + Blepsize] * frac;
			buf[(bpos + i) & nmask] += mixvalue * scale;
			lpIn++;
		}
	}
	inline void mixInImpulseCenter(float *buf, int &bpos, float offset, float scale)
	{
		int lpIn = (int)(B_OVERSAMPLING * offset);
		float frac = offset * B_OVERSAMPLING - lpIn;
		float f1 = 1.0f - frac;
		lpIn *= Blepsize;
		for (int i = 0; i < n; i++) {
			float mixvalue = blepPTR[lpIn] * f1 + blepPTR[lpIn + Blepsize] * frac;
			buf[(bpos + i) & nmask] += mixvalue * scale;
			lpIn++;
		}
	}
	inline float getNextBlep(float *buf, int &bpos)
	{
		bpos = (bpos + 1) & nmask;
		float value = buf[bpos];
		buf[bpos] = 0.0f;

		return value;
	}
};
