/*
	==============================================================================
	This file is part of the MiMi-d synthesizer.

	Copyright © 2013-2014 Filatov Vadim
	Copyright 2024 Ricard Wanderlof

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
//Always feed first then get delayed sample!
template<unsigned int DM> class DelayLine
{
protected:
	float dl[DM];
	int iidx;
	unsigned int length;
	unsigned int mask; // Used for wraparound masking and also for
			   // length - 1 when required in calculations.
public:
	DelayLine()
	{
		iidx = 0;
		length = DM;
		mask = DM - 1;
		zeromem(dl, sizeof(float) * DM);
		// TODO: Verify DM is power-of-two
		//jassert(DM > DMAX);
	}
	inline float feedReturn(float sm)
	{
		dl[iidx] = sm;
		iidx--;
		iidx = iidx & mask;
		return dl[iidx];
	}
	inline void fillZeroes()
	{
		zeromem(dl, DM * sizeof(float));
	}
	virtual void setLength(unsigned int newlength)
	{
		if (newlength >= DM)
			return;
		length = newlength;
		mask = newlength - 1;
	}
};
template<unsigned int DM> class DelayLineRampable: public DelayLine<DM>
{
using DelayLine<DM>::iidx;
using DelayLine<DM>::dl;
using DelayLine<DM>::length;
using DelayLine<DM>::mask;
private:
	float cosramp[DM];
	void generate_cosramp() {
		// create cosine ramp 1 .. 0, inclusive (DM elements)
		for (unsigned int i = 0; i < length; i++)
			// Argument goes from 0 to 1 * (PI/2)
			cosramp[i] = cosf(pi * i / (float) (mask * 2));
			// Linear ramp from 1 to 0 would be:
			//ramp[i] = (mask - i) / (float) mask;
	}
public:
	DelayLineRampable()
	{
		DelayLine<DM>();
		generate_cosramp();
	}
	inline void decayLine()
	{
		// Ramp down from next value to fetch to zero
		// Sortof assumes that the next value to be entered into
		// the delay line will in fact be zero.
		int idxtmp = (iidx + mask) & mask; // next to fetch
		float fetchval = dl[idxtmp];
		// We don't touch the fetchval entry as it is to be scaled
		// by 1, so skip the first value (start loop at 1, and start
		// by decrementing idxtmp).
		for (unsigned int i = 1; i < length; i++) {
			idxtmp--;
			idxtmp &= mask;
			dl[idxtmp] = fetchval * cosramp[i];
		}
	}
	void setLength(unsigned int newlength) override
	{
		DelayLine<DM>::setLength(newlength);
		generate_cosramp();
	}
};
template<unsigned int DM> class DelayLineBoolean
{
private:
	bool dl[DM];
	int iidx;
	unsigned int mask;
public:
	DelayLineBoolean()
	{
		iidx = 0;
		zeromem(dl, sizeof(bool) * DM);
		mask = DM - 1;
	}
	inline bool feedReturn(bool sm)
	{
		dl[iidx] = sm;
		iidx--;
		iidx = iidx & mask;
		return dl[iidx];
	}

};
template<unsigned int DM> class DelayLineInt
{
private:
	int dl[DM];
	int iidx;
	unsigned int mask;
public:
	DelayLineInt()
	{
		iidx = 0;
		zeromem(dl, sizeof(bool) * DM);
		mask = DM - 1;
	}
	inline int feedReturn(int sm)
	{
		dl[iidx] = sm;
		iidx--;
		iidx = iidx & mask;
		return dl[iidx];
	}

};
