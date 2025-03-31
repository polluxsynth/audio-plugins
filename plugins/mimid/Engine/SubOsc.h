/*
	==============================================================================
	This file is part of the MiMi-d synthesizer.

        Copyright © 2013-2014 Filatov Vadim
        Copyright 2023-2024 Ricard Wanderlof
	
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
#include "Antialias.h"

class SubOsc
{
	Antialias &antialias;
	int counter;
	bool state, prevState;
public:
	SubOsc(Antialias &a): antialias(a)
	{
		counter = 0;
		state = prevState = 0;
	}
	~SubOsc()
	{
	}
	inline void setDecimation()
	{
		antialias.setDecimation();
	}
	inline void removeDecimation()
	{
		antialias.removeDecimation();
	}
	inline void processMaster(bool hsr, float hsfrac, int waveformMask)
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

		antialias.mixInImpulseCenter(hsfrac, state ? -1 : 1);
	}
	inline float getValue(int waveformMask)
	{
		float oscmix = 0;
		if (waveformMask & 3) {
			oscmix = state ? 0.5 : -0.5;
			if (waveformMask == 3)
				oscmix += 0.25; // DC offset compensation
		}
		antialias.putSample(oscmix);
		return antialias.getNextSample();
	}
};
