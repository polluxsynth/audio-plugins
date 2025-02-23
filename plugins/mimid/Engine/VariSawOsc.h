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
class VariSawOsc
{
	bool pastBp = true;
	float prevBp;
	float buffer1[Samples * 2];
	const int n, nmask;
	float const *blepPTR;
	float const *blampPTR;
	int bP1;
public:
	VariSawOsc(): n(Samples * 2), nmask(Samples * 2 - 1)
	{
		pastBp = false;
		prevBp = 0;
		bP1 = 0;
		for (int i = 0; i < n; i++)
			buffer1[i] = 0;
		blepPTR = blep;
	}
	~VariSawOsc()
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
	inline void processMaster(float x, float delta, float symm, float bp, float grad, bool waveformReset)
	{
		if (waveformReset) {
			float trans = x - delta;
			float mix = symm >= 0 ?
				trans <= bp ? 0 : (trans - bp) * grad :
		                trans >= bp  ? 1 : trans * grad;
			if (symm > 0 && trans > symm)
				mixInBlampCenter(buffer1, bP1, 1.0f, grad * Samples * delta);
			else if (symm < 0 && trans > bp)
				mixInBlampCenter(buffer1, bP1, 1.0f, -grad * Samples * delta);
			mixInImpulseCenter(buffer1, bP1, 1.0f, mix);
			pastBp = false; // Waveform restarted
			return;
		}
		float summated = delta - (bp - prevBp);
		if (pastBp && x >= 1.0f) {
			x -= 1.0f;
			mixInImpulseCenter(buffer1, bP1, x / delta, 1.0f);
			if (symm > 0)
				mixInBlampCenter(buffer1, bP1, x / delta, grad * Samples * delta);
			else if (symm < 0)
				mixInBlampCenter(buffer1, bP1, x / delta, -grad * Samples * delta);
			pastBp = false;
		}
		if (!pastBp && x >= bp && x - summated <= bp) {
			pastBp = true;
			if (symm > 0) {
				float frac = (x - bp) / summated;
				mixInBlampCenter(buffer1, bP1, frac, -grad * Samples * summated);
			} else if (symm < 0) {
				float frac = (x - bp) / summated;
				mixInBlampCenter(buffer1, bP1, frac, grad * Samples * summated);
			}
		}
		if (pastBp && x >= 1.0f) {
			x -= 1.0f;
			mixInImpulseCenter(buffer1, bP1, x / delta, 1.0f);
			if (symm > 0)
				mixInBlampCenter(buffer1, bP1, x / delta, grad * Samples * delta);
			else if (symm < 0)
				mixInBlampCenter(buffer1, bP1, x / delta, -grad * Samples * delta);
			pastBp = false;
		}
		prevBp = bp;
	}
	inline float getValue(float x, float symm, float bp, float gradient)
	{
		float mix = symm >= 0 ? x <= bp ? 0 : (x - bp) * gradient :
			                x >= bp ? 1 : x * gradient;
		// DC level: symm > 0: half of sawtooth peak, so
		// (1 - symm) * 0.5
		// symm < 0: half of sawtooth peak, so
		// (1 + fabsf(symm)) * 0.5 = (1 - symm) * 0.5
		// total (1 - symm) * 0.5
		mix -= (1 - symm) * 0.5;

		// Instead of subtracting Samples to get to the middle of the
		// BLEP buffer, and then masking with size-1 to keep the
		// offset inside the buffer, we can just XOR the offset with
		// Samples (which corresponds to subtracting (or adding, for
		// that matter) half the buffer size and discarding the carry),
		// since the buffer is 2 * Samples long.
		buffer1[bP1 ^ Samples] += mix;
		return getNextBlep(buffer1, bP1);
	}
	inline void processSlave(float x, float delta, bool hardSyncReset, float hardSyncFrac, float symm, float bp, float grad)
	{
		bool hspass = true;
		float summated = delta - (bp - prevBp);
		if (pastBp && x >= 1.0f) {
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				mixInImpulseCenter(buffer1, bP1, x / delta, 1.0f);
				if (symm > 0)
					mixInBlampCenter(buffer1, bP1, x / delta, grad * Samples * delta);
				else if (symm < 0)
					mixInBlampCenter(buffer1, bP1, x / delta, -grad * Samples * delta);
				pastBp = false;
			} else {
				// if transition did not occur
				x += 1.0f;
				hspass = false;
			}
		}
		if (!pastBp && x >= bp && x - summated <= bp && hspass) {
			pastBp = true;
			float frac = (x - bp) / summated;
			if (!hardSyncReset || (frac > hardSyncFrac)) { // de morgan processed equation
				if (symm > 0)
					mixInBlampCenter(buffer1, bP1, frac, -grad * Samples * summated);
				else if (symm < 0)
					mixInBlampCenter(buffer1, bP1, frac, grad * Samples * summated);
			} else {
				// if transition did not occur
				pastBp = false;
			}
		}
		if (pastBp && x >= 1.0f && hspass) {
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				mixInImpulseCenter(buffer1, bP1, x / delta, 1.0f);
				if (symm > 0)
					mixInBlampCenter(buffer1, bP1, x / delta, grad * Samples * delta);
				else if (symm < 0)
					mixInBlampCenter(buffer1, bP1, x / delta, -grad * Samples * delta);
				pastBp = false;
			} else {
				// if transition did not occur
				x += 1.0f;
			}
		}
		if (hardSyncReset) {
			// Reset the waveform, and restart it at hardSyncFrac
			float fracMaster = delta * hardSyncFrac;
			float trans = x - fracMaster;
			float mix = symm >= 0 ?
				trans <= bp ? 0 : (trans - bp) * grad :
		                trans >= bp  ? 1 : trans * grad;
			if (symm > 0 && trans > symm)
				mixInBlampCenter(buffer1, bP1, hardSyncFrac, grad * Samples * delta);
			else if (symm < 0 && trans > bp)
				mixInBlampCenter(buffer1, bP1, hardSyncFrac, -grad * Samples * delta);
			mixInImpulseCenter(buffer1, bP1, hardSyncFrac, mix);
			pastBp = false;
			// If symmetry is very small, we might get an event
			// in the same sample period as the hard sync event
			x = fracMaster;
			if (x >= bp && x - summated < bp) {
				pastBp = true;
				if (symm > 0) {
					float frac = (x - bp) / summated;
					mixInBlampCenter(buffer1, bP1, frac, -grad * Samples * summated);
				} else if (symm < 0) {
					float frac = (x - bp) / summated;
					mixInBlampCenter(buffer1, bP1, frac, grad * Samples * summated);
				}
			}
		}
		prevBp = bp;
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
		for (int i = 0 ; i < n; i++) {
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
