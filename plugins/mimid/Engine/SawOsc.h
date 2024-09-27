/*
	==============================================================================
	This file is part of the MiMi-d synthesizer.

	Copyright © 2013-2014 Filatov Vadim
	Copyright 2023-2024 Ricard Wanderlof
	
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
class SawOsc
{
	float buffer1[Samples * 2];
	const int n;
	float const *blepPTR;
	int bP1;
public:
	SawOsc(): n(Samples * 2)
	{
		bP1 = 0;
		for (int i = 0; i < n; i++)
			buffer1[i]=0;
		blepPTR = blep;
	}
	~SawOsc()
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
	inline void processMaster(float x, float delta, bool waveformReset)
	{
		if (waveformReset)
		{
			float trans = x - delta;
			mixInImpulseCenter(buffer1, bP1, 1, trans);
			return;
		}
		if (x >= 1.0f)
		{
			x -= 1.0f;
			mixInImpulseCenter(buffer1, bP1, x / delta, 1);
		}
	}
	inline float getValue(float x)
	{
		// Instead of subtracting Samples to get to the middle of the
		// BLEP buffer, and then masking with size-1 to keep the
		// offset inside the buffer, we can just XOR the offset with
		// Samples (which corresponds to subtracting (or adding, for
		// that matter) half the buffer size and discarding the carry),
		// since the buffer is 2 * Samples long.
		buffer1[bP1 ^ Samples] += x - 0.5;
		return getNextBlep(buffer1, bP1);
	}
	inline void processSlave(float x, float delta, bool hardSyncReset, float hardSyncFrac)
	{
		if (x >= 1.0f)
		{
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) //de morgan processed equation
			{
				mixInImpulseCenter(buffer1, bP1, x / delta, 1);
			}
			else
			{
				//if transition do not ocurred
				x += 1;
			}
		}
		if (hardSyncReset)
		{
			float fracMaster = delta * hardSyncFrac;
			float trans = x - fracMaster;
			mixInImpulseCenter(buffer1, bP1, hardSyncFrac, trans);
		}
	}
	inline void mixInImpulseCenter(float *buf, int &bpos, float offset, float scale)
	{
		int lpIn =(int)(B_OVERSAMPLING * offset);
		float frac = offset * B_OVERSAMPLING - lpIn;
		float f1 = 1.0f - frac;
		for (int i = 0 ; i < n; i++)
		{
			float mixvalue = blepPTR[lpIn] * f1 + blepPTR[lpIn + 1] * frac;
			buf[(bpos + i) & (n - 1)] += mixvalue * scale;
			lpIn += B_OVERSAMPLING;
		}
	}
	inline float getNextBlep(float *buf, int &bpos)
	{
		bpos = (bpos + 1) & (n - 1);
		float value = buf[bpos];
		buf[bpos] = 0.0f;

		return value;
	}
};
