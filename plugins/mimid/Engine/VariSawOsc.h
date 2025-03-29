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
	// breakpoint is 0..1 (spikey sawtooth .. standard sawtooth)
	// (Sawtooth is actually inverted when spikey)
	// gradient is normally 1/breakpoint, so
	// breakpoint 0 .. 1 => gradient inf .. 1, but gradient can also
	// be < 1, in which case breakpoint remains at 1, and the saw peak
	// (at right hand edge) starts dropping instead.
	inline void processMaster(float x, float delta, float bp, float grad, bool waveformReset)
	{
		if (waveformReset) {
			float trans = x - delta;
			float mix = x > bp ? 1.0f : x * grad;
			if (trans > bp)
				mixInBlampCenter(buffer1, bP1, 1.0f, -grad * Samples * delta);
			mixInImpulseCenter(buffer1, bP1, 1.0f, mix);
			pastBp = false; // Waveform restarted
			return;
		}
		float summated = delta - (bp - prevBp);
		if (pastBp && x >= 1.0f) {
			x -= 1.0f;
			mixInImpulseCenter(buffer1, bP1, x / delta, grad < 1.0f ? grad : 1.0f);
			if (bp < 1.0f)
				mixInBlampCenter(buffer1, bP1, x / delta, -grad * Samples * delta);
			pastBp = false;
			// We can return early here, becuase there are
			// limits to the gradient and consequently breakpoint
			// which mean that for sane sample rates and osc
			// frequencies, we will never reach the breakpoint in
			// the same sample interval as a waveform reset.
			return;
		}
		if (!pastBp && x >= bp && x - summated <= bp) {
			pastBp = true;
			if (bp < 1.0f) {
				float frac = (x - bp) / summated;
				mixInBlampCenter(buffer1, bP1, frac, grad * Samples * summated);
			}
		}
		if (pastBp && x >= 1.0f) {
			x -= 1.0f;
			mixInImpulseCenter(buffer1, bP1, x / delta, grad < 1.0f ? grad : 1.0f);
			if (bp < 1.0f)
				mixInBlampCenter(buffer1, bP1, x / delta, -grad * Samples * delta);
			pastBp = false;
		}
		prevBp = bp;
	}
	inline float getValue(float x, float bp, float grad)
	{
		float mix = x > bp ? 1.0f : x * grad;
		// DC level: gradient <= 1: half of end of sawtooth peak,
		// so, simply, 0.5 * gradient .
		// gradient > 1: 1 for bp..1, 1/2 for 0..bp, so
		// (1 - bp) + 0.5 * bp = 1 - bp + 0.5 * bp =
		// 1 - 0.5 * bp
		// all in all: gradient <= 1 ? 0.5 * grad : 1 - 0.5 * bp;
		mix -= grad <= 1.0f ? 0.5f * grad : 1.0f - 0.5f * bp;

		// Instead of subtracting Samples to get to the middle of the
		// BLEP buffer, and then masking with size-1 to keep the
		// offset inside the buffer, we can just XOR the offset with
		// Samples (which corresponds to subtracting (or adding, for
		// that matter) half the buffer size and discarding the carry),
		// since the buffer is 2 * Samples long.
		buffer1[bP1 ^ Samples] += mix;
		return getNextBlep(buffer1, bP1);
	}
	inline void processSlave(float x, float delta, bool hardSyncReset, float hardSyncFrac, float bp, float grad)
	{
		bool hspass = true;
		float summated = delta - (bp - prevBp);
		if (pastBp && x >= 1.0f) {
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				mixInImpulseCenter(buffer1, bP1, x / delta, grad < 1.0f ? grad : 1.0f);
				if (bp < 1.0f)
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
				if (bp < 1.0f)
					mixInBlampCenter(buffer1, bP1, frac, grad * Samples * summated);
			} else {
				// if transition did not occur
				pastBp = false;
			}
		}
		if (pastBp && x >= 1.0f && hspass) {
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				mixInImpulseCenter(buffer1, bP1, x / delta, grad < 1.0f ? grad : 1.0f);
				if (bp < 1.0f)
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
			float mix = x > bp ? 1.0f : x * grad;
			if (trans > bp)
				mixInBlampCenter(buffer1, bP1, hardSyncFrac, -grad * Samples * delta);
			mixInImpulseCenter(buffer1, bP1, hardSyncFrac, mix);
			pastBp = false;
			x = fracMaster;
			// Since the maximum gradient limits the breakpoint to
			// more than a sample period, for practical sample
			// rates, we don't need to consider a ramp-to-flatline
			// transition (which would have resulted in a blamp
			// insertion) here.
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
