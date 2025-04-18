/*
  ==============================================================================
  This file is part of the MiMi-d synthesizer,
  originally from Obxd synthesizer.

  Copyright © 2013-2014 Filatov Vadim
  Copyright 2023-2025 Ricard Wanderlof

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
#include "Voice.h"
#include <math.h>

class Filter
{
private:
	float s1, s2, s3, s4;
	float R;
	float R24;

	//24 db variable cutoff slope
	float mmt;
	int mmch = 0;
public:
	float SampleRate;
	float sampleRateInv;
	bool bandPassSw; // LP->BP->HP instead of LP->notch->HP when set
	float mm;
	float unused1, unused2;
	Filter()
	{
		bandPassSw = true;
		mm = 0;
		s1 = s2 = s3 = s4 = 0.0f;
		SampleRate = 44000.0f;
		sampleRateInv = 1.0f / SampleRate;
		R = 1.0f;
		R24 = 0.0f;
		unused1 = 1;
		unused2 = 1;
	}
	void setResponse(float m)
	{
		// m = 0..3 (4 pole down to 1 pole)
		mm = m;
		mmch = (int)mm;
		mmt = mm - mmch;
	}
	inline void setSampleRate(float sr)
	{
		SampleRate = sr;
		sampleRateInv = 1.0f / SampleRate;
	}

	inline float diodePairResistanceApprox(float x)
	{
		//Taylor approx of slightly mismatched diode pair
		return ((((0.0103592f)*x + 0.00920833f)*x + 0.185f)*x + 0.05f )*x + 1.0f;
	}
	//resolve 0-delay feedback
	inline float NR(float sample, float g)
	{
		//calculating feedback non-linear transconducance and compensated for R (-1)
		//Boosting non-linearity
		//Replace 1.035f with 1.0f to avoid self oscillation
		float tCfb = diodePairResistanceApprox(s1 * 0.0876f) - 1.035f;
		//float tCfb = 0;
		//disable non-linearity == digital filter

		//resolve linear feedback
		float y = (sample - 2.0f*(s1*(R+tCfb)) - g*s1  - s2) / (1.0f + g*(2.0f*(R+tCfb) + g));

		//float y = (sample - 2.0f*(s1*(R+tCfb)) - g2*s1  - s2) / (1.0f + g1*(2.0f*(R+tCfb) + g2));

		return y;
	}
	inline float Apply(float sample,float g, float r)
        {
		R = 1.0f - r;
		g = tanf(g *sampleRateInv * pi);
		//float v = (sample - R * s1*2.0f - g2*s1 - s2) / (1.0f + R*g1*2.0f + g1*g2);
		float v = NR(sample, g);

		float y1 = v*g + s1;
		s1 = v*g + y1;

		float y2 = y1*g + s2;
		s2 = y1*g + y2;

		float mc;
		if (!bandPassSw)
			mc = (1.0f - mm)*y2 + (mm)*v;
		else
			mc = 2 * (mm < 0.5f ?
				  ((0.5f - mm) * y2 + mm * y1):
				  ((1.0f - mm) * y1 + (mm - 0.5f) * v));
		return mc;
        }
	inline float NR24(float sample, float ml, float lpc)
	{
		float S = (lpc*(lpc*(lpc*s1 + s2) + s3) + s4) * ml;
		// Alternate position for soft clip to avoid oscillation blowup.
		// By clipping S, we only clip the feedback loop, i.e. no
		// distortion when resonance is at 0. The amount of clipping is
		// so small that it has negligible impact on the signal through
		// the filter though, and clipping S gives a slight pitch
		// offset (< 1 cent) when filter is oscillating.
		// S *= 1 - S * S * 0.017784f;
		float G = lpc * lpc * lpc * lpc;
		float y = (sample - R24 * S) / (1.0f + R24 * G);
		return y;
	}
	inline float tan_approx(float x)
	{
		// Truncated Taylor series
		// return x + 1.0/3 * x*x*x; // + 2.0/15 * x*x*x*x*x + 17.0/315 * x*x*x*x*x*x*x;
		return x * (1.0f + 1.0f/3.0f * x * x);
	}
	inline float Apply4Pole(float sample, float g, float r)
	{
		R24 = 4.2f * r;
		g = (float)tan_approx(g *sampleRateInv * pi);

		// Gain compensation for varying resonance values
		sample *= 1.0f + R24 * 0.45f;

		float ml = 1.0f / (1.0f + g);
		float lpc = ml * g;
		float y0 = NR24(sample, ml, lpc);

		// Subtle amplitude limiting to avoid filter blowing up
		// when resonating, using Taylor approximation truncated
		// after third degree term, with appropriate scaling.
		// The resulting distortion is so subtle that it hardly
		// qualifies as distortion for the signal through the filter.
		// The constant in the squared term sets the amplitude of
		// the filter oscillation, and also limits the amount of
		// resonance that can be achieved. The amplitude scales
		// linearly with the inverse of the square of the constant,
		// e.g. to tripple the oscillation amplitude, divide the
		// constant by 3**2. 0.001010 gives an oscillation amplitude
		// of approximately the same level as the ordinary signal
		// through the filter (amplitude 3 for a single oscillator).
		// Derivation:
		// Soft clipping by multiplying sample value by scale,
		// taking atan, and then dividing by scale to keep
		// unclipped amplitude constant.
		// y0 = atan(y0 * scale) / scale
		// Truncated Taylor around x=0: atan(x) = x - (1/3)*x^3, so
		// y0 = (y0 * scale - (1/3)*(y0 * scale)^3) / scale
		// y0 = y0 - (1/3)*y0^3*scale^2
		// y0 = y0 * (1 - (1/3)*y0^2 * scale^2)
		// y0 *= 1 - (1/3)*y0^2 * scale^2
		// Calculate (1/3) * scale^2 as a single constant scale':
		// y0 *= 1 - y0^2 * scale'
		y0 *= 1.0f - y0 * y0 * 0.001010f;

		// Four single pole filter stages
		float y1 = tptlpc(s1, y0, lpc);
		float y2 = tptlpc(s2, y1, lpc);
		float y3 = tptlpc(s3, y2, lpc);
		float y4 = tptlpc(s4, y3, lpc);

		//return y0 - 4.0f*y1 + 6.0f*y2 - 4.0f*y3 + y4; // HPF output

		float mc;
		// fade between adjacent cutoff slopes
		// (24 - 18 - 12 - 6 dB/oct)
		switch(mmch)
		{
		case 0:
			mc = ((1.0f - mmt) * y4 + mmt * y3);
			break;
		case 1:
			mc = ((1.0f - mmt) * y3 + mmt * y2);
			break;
		case 2:
			mc = ((1.0f - mmt) * y2 + mmt * y1);
			break;
		case 3:
			mc = y1;
			break;
		default:
			mc = 0;
			break;
		}
		return mc;
	}
};
