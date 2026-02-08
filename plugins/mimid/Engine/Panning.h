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
#include <climits>

class Panning
{
public:
#define PAN_CENTER 0
#define PAN_LEFT -1
#define PAN_RIGHT 1
	int position; // -1 = L, 0 = C, +1 = R
	float panSpread; // Random per-voice value 0..1
	float lPanning;	// runtime pan value left ch
	float rPanning;	// runtime pan value right ch

	// panSpreadAmt is amount random spread
	// unisonPanAmt is amount of spread in dual mode
	void setPanning(float panSpreadAmt, float unisonPanAmt)
	{
		// pannings are 0..1, with 0.5 being center, whereas
		// panSpread is 0..1 and val is 0..1
		if (position == PAN_CENTER) {
			lPanning = (panSpread - 0.5f) * panSpreadAmt + 0.5f;
			rPanning = 1.0f - lPanning;
		} else {
			// Since two voices will be playing, we should
			// principally reduce both L and R panning values
			// by ~6 dB = 0.5x . However, since the voices are
			// not identical, go for RMS addition, so -3 dB = 0.71x
			//
			// Also, preserve panSpreadAmt when unisonPanAmt == 0
			// When unisonPanAmt = 1, panSpreadAmt can adjust
			// panning spread from complete left (when 0) and
			// left .. center (when 1). Then, scale linearly
			// for both panSpreadAmt and unisonPanAmt when
			// they have intermediate values.
			float panspread_scaling = 1.0f - 0.5f * unisonPanAmt;
			float panamt_scaling = 1.0f - (1.0f - panSpreadAmt) * unisonPanAmt;
			float panamt_scaling_tot = unisonPanAmt * (1.0f - panSpreadAmt) + panSpreadAmt;
			// Addiitonally, the wider the unison image is, linearly
			// drop the total volume by up to 3 dB (0.71x).
			float total_scaling = 0.71f - unisonPanAmt * (0.71f - 0.71f * 0.71f);
			lPanning = (position * (panSpread * panspread_scaling * panamt_scaling - 0.5f) * panamt_scaling_tot + 0.5f) * total_scaling;
			rPanning = total_scaling - lPanning;
		}
	}
};

template <int S> class Pannings
{
public:
	Panning pannings[S];
	float panSpreadAmt;
	float unisonPanAmt;

	Panning& operator[](int voiceNumber)
	{
		return pannings[voiceNumber];
	}
	void updatePanning(int voiceNumber)
	{
		pannings[voiceNumber].setPanning(panSpreadAmt, unisonPanAmt);
	}
	void updatePannings()
	{
		for (int i = 0; i < S; i++)
			updatePanning(i);
	}
};
