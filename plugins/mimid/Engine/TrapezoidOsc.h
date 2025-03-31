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
#include "Antialias.h"

class TrapezoidOsc
{
	bool falling;
	float prevSymm;
	Antialias antialias;
public:
	TrapezoidOsc(): antialias()
	{
		falling = false;
		prevSymm = 0;
	}
	~TrapezoidOsc()
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
	inline void processMaster(float x, float delta, float symm, float riseGrad, float fallGrad, bool waveformReset)
	{
		if (waveformReset) {
			float trans = x - delta;
			float mix = trans <= symm ? trans * riseGrad : (trans - 1.0f) * fallGrad;
			if (trans > symm)
				antialias.mixInBlampCenter(1.0f, (riseGrad - fallGrad) * Samples * delta);
			antialias.mixInImpulseCenter(1.0f, mix);
			falling = false;
			return;
		}
		float summated = delta - (symm - prevSymm);
		if (falling && x >= 1.0f) {
			x -= 1.0f;
			antialias.mixInBlampCenter(x / delta, (fallGrad - riseGrad) * Samples * delta);
			falling = false;
		}
		if (!falling && x >= symm && x - summated < symm) {
			falling = true;
			float frac = (x - symm) / summated;
			antialias.mixInBlampCenter(frac, (riseGrad - fallGrad) * Samples * summated);
		}
		if (falling && x >= 1.0f) {
			x -= 1.0f;
			antialias.mixInBlampCenter(x / delta, (fallGrad - riseGrad) * Samples * delta);
			falling = false;
		}
		prevSymm = symm;
	}
	inline float getValue(float x, float symm, float riseGrad, float fallGrad)
	{
		float mix = x <= symm ? x * riseGrad : (x - 1.0f) * fallGrad;
		mix -= 0.5f; // DC level is always same regardless of symmetry
		antialias.putSample(mix);
		return antialias.getNextSample();
	}
	inline void processSlave(float x, float delta, bool hardSyncReset, float hardSyncFrac, float symm, float riseGrad, float fallGrad)
	{
		float summated = delta - (symm - prevSymm);
		bool hspass = true;
		if (falling && x >= 1.0f) {
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				antialias.mixInBlampCenter(x / delta, (fallGrad - riseGrad) * Samples * delta);
				falling = false;
			} else {
				x += 1.0f;
				hspass = false;
			}
		}
		if (!falling && x >= symm && x - summated < symm && hspass) {
			float frac = (x - symm) / summated;
			if (!hardSyncReset || (frac > hardSyncFrac)) { // de morgan processed equation
				falling = true;
				antialias.mixInBlampCenter(frac, (riseGrad - fallGrad) * Samples * summated);
			}
		}
		if (falling && x >= 1.0f && hspass) {
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				antialias.mixInBlampCenter(x / delta, (fallGrad - riseGrad) * Samples * delta);
				falling = false;
			} else {
				// if transition did not occur
				x += 1.0f;
			}
		}
		if (hardSyncReset)
		{
			// Reset the waveform, and restart it at hardSyncFrac
			float fracMaster = delta * hardSyncFrac;
			float trans = x - fracMaster;
			float mix = trans <= symm ? trans * riseGrad : (trans - 1.0f) * fallGrad;
			if (trans > symm)
				antialias.mixInBlampCenter(hardSyncFrac, (fallGrad - riseGrad) * Samples * delta);
			antialias.mixInImpulseCenter(hardSyncFrac, mix);
			falling = false;
			// If symmetry is very small, we might get an event
			// in the same sample period as the hard sync event
			x = fracMaster;
			if (x >= symm && x - summated < symm) {
				falling = true;
				float frac = (x - symm) / summated;
				antialias.mixInBlampCenter(frac, (riseGrad - fallGrad) * Samples * summated);
			}

		}
		prevSymm = symm;
	}
};
