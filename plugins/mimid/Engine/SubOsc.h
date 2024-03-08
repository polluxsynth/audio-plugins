/*
	==============================================================================
	This file is part of the MiMi-a synthesizer.

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
#include "BlepData.h"
class SubOsc 
{
	DelayLine<Samples> del1;
	float buffer1[Samples*2];
	const int hsam;
	const int n;
	float const * blepPTR;
	int bP1;
	int counter;
	bool state, prevState;
public:
	SubOsc() : hsam(Samples)
		, n(Samples*2)
	{
	//	del1 = new DelayLine(hsam);
		bP1=0;
		//buffer1= new float[n];
		for(int i = 0 ; i < n ; i++)
			buffer1[i]=0;
		blepPTR = blep;
		counter = 0;
		state = prevState = 0;
	}
	~SubOsc()
	{
	//	delete buffer1;
	//	delete del1;
	}
	inline void setDecimation()
	{
		blepPTR = blepd2;
	}
	inline void removeDecimation()
	{
		blepPTR = blep;
	}
	inline float aliasReduction()
	{
		return -getNextBlep(buffer1,bP1);
	}
	inline void processMaster(bool hsr,float hsfrac,int waveformMask)
	{
		if (!hsr)
			return;
		// advance counter, change state depending on waveoform mask
		counter++;
		counter &= 3;
                state = (counter & waveformMask) != 0;
		if (state == prevState)
			return;
		prevState = state;

		mixInImpulseCenter(buffer1,bP1,hsfrac, state ? -1 : 1);
	}
	inline float getValue(int waveformMask)
	{
		float oscmix = 0;
		if (waveformMask & 3) {
			oscmix = state ? 0.5 : -0.5;
			if (waveformMask == 3)
				oscmix += 0.25; // DC offset compensation
		}
		return del1.feedReturn(oscmix);
	}
	inline void mixInImpulseCenter(float * buf,int& bpos,float offset, float scale) 
	{
		int lpIn =(int)(B_OVERSAMPLING*(offset));
		float frac = offset * B_OVERSAMPLING - lpIn;
		float f1 = 1.0f-frac;
		for(int i = 0 ; i < Samples;i++)
		{
			float mixvalue = (blepPTR[lpIn]*f1+blepPTR[lpIn+1]*(frac));
			buf[(bpos+i)&(n-1)]  += mixvalue*scale;
			lpIn += B_OVERSAMPLING;
		}
		for(int i = Samples ; i <n;i++)
		{
			float mixvalue = (blepPTR[lpIn]*f1+blepPTR[lpIn+1]*(frac));
			buf[(bpos+i)&(n-1)]  -= mixvalue*scale;
			lpIn += B_OVERSAMPLING;
		}
	}
	inline float getNextBlep(float* buf,int& bpos) 
	{
		buf[bpos]= 0.0f;
		bpos++;

		// Wrap pos
		bpos&=(n-1);
		return buf[bpos];
	}
};
