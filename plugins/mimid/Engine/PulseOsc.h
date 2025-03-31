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

class PulseOsc
{
	bool pw1t;
	float prevPulseWidth;
	Antialias antialias;
public:
	PulseOsc(): antialias()
	{
		pw1t = false;
	}
	~PulseOsc()
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
	inline void processMaster(float x, float delta, float pulseWidth, bool waveformReset)
	{
		if (waveformReset) {
			float trans = pw1t ? 1.0f : 0.0f;
			antialias.mixInImpulseCenter(1.0f, trans);
			pw1t = false;
			return;
		}
		float summated = delta - (pulseWidth - prevPulseWidth);
		if (pw1t && x >= 1.0f) {
			x -= 1.0f;
			antialias.mixInImpulseCenter(x / delta, 1.0f);
			pw1t = false;
		}
		if (!pw1t && (x >= pulseWidth) && (x - summated <= pulseWidth)) {
			pw1t = true;
			float frac = (x - pulseWidth) / summated;
			antialias.mixInImpulseCenter(frac, -1.0f);
		}
		if (pw1t && x >= 1.0f) {
			x -= 1.0f;
			antialias.mixInImpulseCenter(x / delta, 1.0f);
			pw1t = false;
		}
		prevPulseWidth = pulseWidth;
	}
	inline float getValue(float x, float pulseWidth)
	{
		float oscmix;

		if (x >= pulseWidth)
			oscmix = 1 - (0.5f - pulseWidth) - 0.5f;
		else
			oscmix = -(0.5f - pulseWidth) - 0.5f;
		antialias.putSample(oscmix);
                return antialias.getNextSample();
	}
	inline void processSlave(float x, float delta, bool hardSyncReset, float hardSyncFrac, float pulseWidth)
	{
		float summated = delta - (pulseWidth - prevPulseWidth);

		if (pw1t && x >= 1.0f)
		{
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				antialias.mixInImpulseCenter(x / delta, 1.0f);
				pw1t = false;
			} else {
				x += 1.0f;
			}
		}

		if (!pw1t && (x >= pulseWidth) && (x - summated <= pulseWidth))
		{
			pw1t = true;
			float frac = (x - pulseWidth) / summated;
			if (!hardSyncReset || (frac > hardSyncFrac)) { // de morgan processed equation
				// transition to 1
				antialias.mixInImpulseCenter(frac, -1.0f);
			} else {
				// if transition did not occur
				pw1t = false;
			}
		}
		if (pw1t && x >= 1.0f)
		{
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				antialias.mixInImpulseCenter(x / delta, 1.0f);
				pw1t = false;
			} else {
				x += 1.0f;
			}
		}

		if (hardSyncReset) {
			if (pw1t)
				antialias.mixInImpulseCenter(hardSyncFrac, 1.0f);
			pw1t = false;
			x = hardSyncFrac * delta;
			// If pulsewidth is very small, we might get an event
			// in the same sample period as the hard sync event
			if ((x >= pulseWidth) && (x - summated <= pulseWidth)) {
				pw1t = true;
				float frac = (x - pulseWidth) / summated;
				antialias.mixInImpulseCenter(frac, -1.0f);
			}
		}

		prevPulseWidth = pulseWidth;
	}
};
