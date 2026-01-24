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

struct Panning
{
public:
	float panSpread; // Random per-voice value -0.5..+0.5
	float lPanning;	// runtime pan value left ch
	float rPanning;	// runtime pan value right ch

	void setPanning(float panSpreadAmt)
	{
		// pannings are 0..1, with 0.5 being center, whereas
		// panSpread is [-0.5..+0.5] and val is 0..1
		lPanning = panSpread * panSpreadAmt + 0.5f;
		rPanning = 1.0f - lPanning;
	}
};

template <int S> struct Pannings
{
	Panning pannings[S];
	float panSpreadAmt;

	Panning& operator[](int voiceNumber)
	{
		return pannings[voiceNumber];
	}
	void updatePanning(int voiceNumber)
	{
		pannings[voiceNumber].setPanning(panSpreadAmt);
	}
	void updatePannings()
	{
		for (int i = 0; i < S; i++)
			updatePanning(i);
	}
};
