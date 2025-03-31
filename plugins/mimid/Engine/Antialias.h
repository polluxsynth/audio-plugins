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

class Antialias
{
	float buffer[Samples * 2];
	const int n, nmask;
	float const *blepPTR;
	float const *blampPTR;
	int bpos;

public:
	Antialias(): n(Samples * 2), nmask(Samples * 2 - 1)
	{
		bpos = 0;
		for (int i = 0; i < n; i++)
			buffer[i] = 0;
		blepPTR = blep;
		blampPTR = blamp;
	}
	~Antialias()
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
	inline void mixInBlampCenter(float offset, float scale)
	{
		int lpIn = (int)(B_OVERSAMPLING * offset);
		float frac = offset * B_OVERSAMPLING - lpIn;
		float f1 = 1.0f - frac;
		lpIn *= Blepsize;
		for (int i = 0; i < n; i++) {
			float mixvalue = blampPTR[lpIn] * f1 + blampPTR[lpIn + Blepsize] * frac;
			buffer[(bpos + i) & nmask] += mixvalue * scale;
			lpIn++;
		}
	}
	inline void mixInImpulseCenter(float offset, float scale) 
	{
		int lpIn = (int)(B_OVERSAMPLING * offset);
		float frac = offset * B_OVERSAMPLING - lpIn;
		float f1 = 1.0f - frac;
		lpIn *= Blepsize;
		for (int i = 0; i < n; i++) {
			float mixvalue = blepPTR[lpIn] * f1 + blepPTR[lpIn + Blepsize] * frac;
			buffer[(bpos + i) & nmask] += mixvalue * scale;
			lpIn++;
		}
	}
	// Put new sample in buffer, to be fetched later, with potential
	// blep/blamp, when getNextSample() is called.
	// Instead of subtracting Samples to get to the middle of the
	// buffer, and then masking with size-1 to keep the
	// offset inside the buffer, we can just XOR the offset with
	// Samples (which corresponds to subtracting (or adding, for
	// that matter) half the buffer size and discarding the carry),
	// since the buffer is 2 * Samples long.
	inline void putSample(float sample)
	{
		 buffer[bpos ^ Samples] += sample;
	}
	// Get next sample from buffer
	// Note: Also advances the buffer pointer to next position
	inline float getNextSample()
	{
		bpos = (bpos + 1) & nmask;
		float value = buffer[bpos];
		buffer[bpos] = 0.0f; // clear buffer as data is fetched

		return value;
	}
};
