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
class PulseOsc
{
	bool pw1t;
	float buffer1[Samples * 2];
	const int n, nmask;
	float const *blepPTR;
	int bP1;
public:
	PulseOsc(): n(Samples * 2), nmask(Samples * 2 - 1)
	{
		pw1t = false;
		bP1 = 0;
		for (int i = 0; i < n; i++)
			buffer1[i] = 0;
		blepPTR = blep;
	}
	~PulseOsc()
	{
	}
	inline void setDecimation()
	{
		blepPTR = blepd2;
	}
	inline void removeDecimation()
	{
		blepPTR = blep;
	}
	inline void processMaster(float x, float delta, float pulseWidth, float pulseWidthWas, bool waveformReset)
	{
		if (waveformReset) {
			float trans = pw1t ? 1.0f : 0.0f;
			mixInImpulseCenter(buffer1, bP1, 1.0f, trans);
			pw1t = false;
			return;
		}
		float summated = delta - (pulseWidth - pulseWidthWas);
		if (pw1t && x >= 1.0f) {
			x -= 1.0f;
			if (pw1t)
				mixInImpulseCenter(buffer1, bP1, x / delta, 1.0f);
			pw1t = false;
		}
		if (!pw1t && (x >= pulseWidth) && (x - summated <= pulseWidth)) {
			pw1t = true;
			float frac = (x - pulseWidth) / summated;
			mixInImpulseCenter(buffer1, bP1, frac, -1.0f);
		}
		if (pw1t && x >= 1.0f) {
			x -= 1.0f;
			if (pw1t)
				mixInImpulseCenter(buffer1, bP1, x / delta, 1.0f);
			pw1t = false;
		}

	}
	inline float getValue(float x, float pulseWidth)
	{
		float oscmix;

		if (x >= pulseWidth)
			oscmix = 1 - (0.5f - pulseWidth) - 0.5f;
		else
			oscmix = -(0.5f - pulseWidth) - 0.5f;
                // Instead of subtracting Samples to get to the middle of the
                // BLEP buffer, and then masking with size-1 to keep the
                // offset inside the buffer, we can just XOR the offset with
                // Samples (which corresponds to subtracting (or adding, for
                // that matter) half the buffer size and discarding the carry),
                // since the buffer is 2 * Samples long.
                buffer1[bP1 ^ Samples] += oscmix;
                return getNextBlep(buffer1, bP1);
	}
	inline void processSlave(float x, float delta, bool hardSyncReset, float hardSyncFrac, float pulseWidth, float pulseWidthWas)
	{
		float summated = delta - (pulseWidth - pulseWidthWas);

		if (pw1t && x >= 1.0f)
		{
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				if (pw1t)
					mixInImpulseCenter(buffer1, bP1, x / delta, 1.0f);
				pw1t = false;
			} else {
				x += 1.0f;
			}
		}

		if (!pw1t && (x >= pulseWidth) && (x - summated <= pulseWidth))
		{
			pw1t = true;
			float frac = (x - pulseWidth) / summated;
			if (!hardSyncReset || (frac > hardSyncFrac)) { // de morgan processed equation
				// transition to 1
				mixInImpulseCenter(buffer1, bP1, frac, -1.0f);
			} else {
				// if transition did not occur
				pw1t = false;
			}
		}
		if (pw1t && x >= 1.0f)
		{
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				if (pw1t)
					mixInImpulseCenter(buffer1, bP1, x / delta, 1.0f);
				pw1t = false;
			} else {
				x += 1.0f;
			}
		}

		if (hardSyncReset) {
			// float fracMaster = delta * hardSyncFrac;
			float trans = pw1t ? 1.0f : 0.0f;
			mixInImpulseCenter(buffer1 ,bP1, hardSyncFrac, trans);
			pw1t = false;
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
