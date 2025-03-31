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

class SawOsc
{
	Antialias antialias;
public:
	SawOsc(): antialias()
	{
	}
	~SawOsc()
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
	inline void processMaster(float x, float delta, bool waveformReset)
	{
		if (waveformReset) {
			float trans = x - delta;
			antialias.mixInImpulseCenter(1.0f, trans);
			return;
		}
		if (x >= 1.0f) {
			x -= 1.0f;
			antialias.mixInImpulseCenter(x / delta, 1.0f);
		}
	}
	inline float getValue(float x)
	{
		antialias.putSample(x - 0.5f);
		return antialias.getNextSample(); // (also bumps buffer pointer)
	}
	inline void processSlave(float x, float delta, bool hardSyncReset, float hardSyncFrac)
	{
		if (x >= 1.0f) {
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				antialias.mixInImpulseCenter(x / delta, 1.0f);
			} else {
				// if transition do not ocurred
				x += 1.0f;
			}
		}
		if (hardSyncReset) {
			float fracMaster = delta * hardSyncFrac;
			float trans = x - fracMaster;
			antialias.mixInImpulseCenter(hardSyncFrac, trans);
		}
	}
};
