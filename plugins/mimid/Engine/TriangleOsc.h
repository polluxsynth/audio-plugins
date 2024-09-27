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
class TriangleOsc 
{
	bool fall;
	float buffer1[Samples * 2];
	const int n;
	float const *blepPTR;
	float const *blampPTR;

	int bP1,bP2;
public:
	TriangleOsc(): n(Samples * 2)
	{
		fall = false;
		bP1 = bP2 = 0;
		for(int i = 0; i < n; i++)
			buffer1[i] = 0;
		blepPTR = blep;
		blampPTR = blamp;
	}
	~TriangleOsc()
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
	inline void processMaster(float x, float delta, bool waveformReset)
	{
		if (waveformReset)
		{
			float trans = x - delta;
			float mix = trans < 0.5 ? 2 * trans - 0.5 : 1.5 - 2 * trans;
			if (trans > 0.5)
				mixInBlampCenter(buffer1, bP1, 1,- 4 * Samples * delta);
			mixInImpulseCenter(buffer1, bP1, 1, mix + 0.5);
			return;
		}
		if (x >= 1.0)
		{
			x -= 1.0;
			mixInBlampCenter(buffer1, bP1, x / delta,- 4 * Samples * delta);
		}
		if (x >= 0.5 && x - delta < 0.5)
		{
			mixInBlampCenter(buffer1, bP1, (x - 0.5) / delta, 4 * Samples * delta);
		}
		if (x >= 1.0)
		{
			x-=1.0;
			mixInBlampCenter(buffer1, bP1, x / delta, -4 * Samples * delta);
		}
	}
	inline float getValue(float x)
	{
		float mix = x < 0.5 ? 2 * x - 0.5 : 1.5 -2 * x;
		// Instead of subtracting Samples to get to the middle of the
		// BLEP buffer, and then masking with size-1 to keep the
		// offset inside the buffer, we can just XOR the offset with
		// Samples (which corresponds to subtracting (or adding, for
		// that matter) half the buffer size and discarding the carry),
		// since the buffer is 2 * Samples long.
		buffer1[bP1 ^ Samples] += mix;
		return getNextBlep(buffer1, bP1);
	}
	inline void processSlave(float x, float delta, bool hardSyncReset, float hardSyncFrac)
	{
		bool hspass = true;
		if (x >= 1.0)
		{
			x -= 1.0;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) //de morgan processed equation
			{
				mixInBlampCenter(buffer1, bP1, x / delta, -4 * Samples * delta);
			}
			else
			{
				x += 1;
				hspass = false;
			}
		}
		if (x >= 0.5 && x - delta < 0.5 && hspass)
		{
			float frac = (x - 0.5) / delta;
			if (!hardSyncReset || (frac > hardSyncFrac)) //de morgan processed equation
			{
				mixInBlampCenter(buffer1, bP1, frac, 4 * Samples * delta);
			}
		}
		if (x >= 1.0 && hspass)
		{
			x-=1.0;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) //de morgan processed equation
			{
				mixInBlampCenter(buffer1, bP1, x / delta, -4 * Samples * delta);
			}
			else
			{
				//if transition did not occur
				x += 1;
			}
		}
		if (hardSyncReset)
		{
			float fracMaster = delta * hardSyncFrac;
			float trans = x - fracMaster;
			float mix = trans < 0.5 ? 2 * trans - 0.5 : 1.5 - 2 * trans;
			if (trans > 0.5)
				mixInBlampCenter(buffer1, bP1, hardSyncFrac, -4 * Samples * delta);
			mixInImpulseCenter(buffer1, bP1, hardSyncFrac, mix + 0.5);
		}
	}
	inline void mixInBlampCenter(float *buf,int &bpos,float offset, float scale)
	{
		int lpIn = (int)(B_OVERSAMPLING * offset);
		float frac = offset * B_OVERSAMPLING - lpIn;
		float f1 = 1.0f - frac;
		lpIn *= Blepsize;
		for (int i = 0; i < n; i++)
		{
			float mixvalue = blampPTR[lpIn] * f1 + blampPTR[lpIn + Blepsize] * frac;
			buf[(bpos + i) & (n - 1)] -= mixvalue * scale;
			lpIn++;
		}
	}
	inline void mixInImpulseCenter(float *buf, int &bpos, float offset, float scale) 
	{
		int lpIn = (int)(B_OVERSAMPLING * offset);
		float frac = offset * B_OVERSAMPLING - lpIn;
		float f1 = 1.0f - frac;
		lpIn *= Blepsize;
		for (int i = 0; i < Samples; i++)
		{
			float mixvalue = blepPTR[lpIn] * f1 + blepPTR[lpIn + Blepsize] * frac;
			buf[(bpos + i) & (n - 1)] -= mixvalue * scale;
			lpIn++;
		}
		for (int i = Samples; i < n; i++)
		{
			float mixvalue = blepPTR[lpIn] * f1 + blepPTR[lpIn + Blepsize] * frac;
			buf[(bpos + i) & (n - 1)] += mixvalue * scale;
			lpIn++;
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
