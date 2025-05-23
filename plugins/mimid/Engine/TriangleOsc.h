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

class TriangleOsc 
{
	bool fall;
	Antialias &antialias;
public:
	TriangleOsc(Antialias &a): antialias(a)
	{
		fall = false;
	}
	~TriangleOsc()
	{
	}
	inline void processMaster(float x, float delta, bool waveformReset)
	{
		if (waveformReset) {
			float trans = x - delta;
			float mix = trans < 0.5f ? 2.0f * trans - 0.5f : 1.5f - 2.0f * trans;
			if (trans > 0.5f)
				antialias.mixInBlampCenter(1.0f, -4.0f * Samples * delta);
			antialias.mixInImpulseCenter(1.0f, mix + 0.5f);
			return;
		}
		if (x >= 1.0f) {
			x -= 1.0f;
			antialias.mixInBlampCenter(x / delta, -4.0f * Samples * delta);
		}
		if (x >= 0.5f && x - delta < 0.5f) {
			antialias.mixInBlampCenter((x - 0.5f) / delta, 4.0f * Samples * delta);
		}
		if (x >= 1.0f) {
			x -= 1.0f;
			antialias.mixInBlampCenter(x / delta, -4.0f * Samples * delta);
		}
	}
	inline float getValue(float x)
	{
		float mix = x < 0.5f ? 2.0f * x - 0.5f : 1.5f - 2.0f * x;
		antialias.putSample(mix);
		return antialias.getNextSample();
	}
	inline void processSlave(float x, float delta, bool hardSyncReset, float hardSyncFrac)
	{
		bool hspass = true;
		if (x >= 1.0f) {
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				antialias.mixInBlampCenter(x / delta, -4.0f * Samples * delta);
			} else {
				x += 1.0f;
				hspass = false;
			}
		}
		if (x >= 0.5f && x - delta < 0.5f && hspass) {
			float frac = (x - 0.5f) / delta;
			if (!hardSyncReset || (frac > hardSyncFrac)) // de morgan processed equation
				antialias.mixInBlampCenter(frac, 4.0f * Samples * delta);
		}
		if (x >= 1.0f && hspass) {
			x -= 1.0f;
			if (!hardSyncReset || (x / delta > hardSyncFrac)) { // de morgan processed equation
				antialias.mixInBlampCenter(x / delta, -4.0f * Samples * delta);
			} else {
				// if transition did not occur
				x += 1.0f;
			}
		}
		if (hardSyncReset)
		{
			float fracMaster = delta * hardSyncFrac;
			float trans = x - fracMaster;
			float mix = trans < 0.5f ? 2.0f * trans - 0.5f : 1.5f - 2.0f * trans;
			if (trans > 0.5f)
				antialias.mixInBlampCenter(hardSyncFrac, -4.0f * Samples * delta);
			antialias.mixInImpulseCenter(hardSyncFrac, mix + 0.5f);
		}
	}
};
