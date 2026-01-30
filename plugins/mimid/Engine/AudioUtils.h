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

// Some handy constants

const float pi = 3.14159265358979323846f; // Replaces juce::float_Pi
const float sq2_12 = 1.0594630943592953f;

const float dc = 1e-18;
const float ln2 = 0.69314718056f;
const float mult = ln2 / 12.0;

// Replacements for Juce library functions

#if 0 // Use alternate implementation to correctly round negative numbers
inline int roundToInt(float f)
{
	return (int) (f + 0.5f);
}
#else
// Alternate implementation:
// Explained in https://stackoverflow.com/questions/17035464
// Basically, add a large value to make the mantissa a whole number, then
// interpret as an int and subtract the offset and exponent.
// Old trick that was very efficient on old Pentiums but which might not
// necessarily be faster now with modern CPUs.
// Note that this alternate implementation correctly rounds negative
// numbers with the exception of hitting exactly 0.5, when it rounds down
// instead of up.
inline int roundToInt(float val)
{
	union { float f; int32_t i; } u;
	u.f = val + 12582912.0f;
	return u.i - 0x4b400000;
}
#endif

inline float minf(const float a, const float b) noexcept
{
	return (a < b) ? a : b;
}

inline float maxf(const float a, const float b) noexcept
{
	return (a > b) ? a : b;
}

inline float limitf(const float val, const float low, const float high) noexcept
{
	return (val < low) ? low : (val > high) ? high : val;
}	

inline void zeromem(void *memory, size_t numBytes) noexcept
{
	memset(memory, 0, numBytes);
}

#ifdef __GNUC__ // Only works if compiler supports type punning
#include "FastExp.h"
#endif

inline static float getPitch(float index)
{
#ifdef __GNUC__
   return 440 * fast_exp2f12(index);
#else
   return 440 * expf(mult * index);
#endif
};

// Go from freq in Hz to note number
inline static float getNote(float freq)
{
	return log(freq / 440) / log(2) * 12;
}

// TPT LPF w/ cutoff supplied in Hz, but no pre-warping
inline static float tptlpupw(float & state , float inp , float cutoff , float srInv)
{
	cutoff = (cutoff * srInv)*pi;
	float v = (inp - state) * cutoff / (1 + cutoff);
	float res = v + state;
	state = res + v;
	return res;
}

// TPT LPF w/ cutoff pre-warping
inline static float tptlp(float& state,float inp,float cutoff,float srInv)
{
	cutoff = tan(cutoff * (srInv)* pi) ;
	float v = (inp - state) * cutoff / (1 + cutoff);
	float res = v + state;
	state = res + v;
	return res;
};

// TPT LPF w/ already pre-warped cutoff
inline static float tptpc(float& state,float inp,float cutoff)
{
	float v = (inp - state) * cutoff / (1 + cutoff);
	float res = v + state;
	state = res + v;
	return res;
}

// TPT LPF, supplying lpc = cutoff / (1 + cutoff)
inline static float tptlpc(float &state, float inp, float lpc)
{
	float v = (inp - state) * lpc;
	float res = v + state;
	state = res + v;
	return res;
}

inline static float linsc(float param,const float min,const float max)
{
	 return (param * 0.1) * (max - min) + min;
}

inline static float logsc(float param, const float min,const float max,const float rolloff = 19.0f)
{
	return ((expf(param * 0.1 * logf(rolloff+1)) - 1.0f) / (rolloff)) * (max-min) + min;
}

inline static float timesc(float param, const float min, const float max)
{
	param *= 0.1;
	param *= param * param * param * param; // param ** 5
	return param * (max - min) + min;
}
