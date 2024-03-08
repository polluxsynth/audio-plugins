/*
	==============================================================================
        This file is part of the MiMi-d synthesizer.

	Simple ramp up triggered by key sync, to be used for delayed LFO.

        Copyright 2024 Ricard Wanderlof

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
class DelayEnvelope
{
private:
	float phase; // 0 -> 1
	float phaseInc;
	float spread;
	float sampleRate;
	float sampleRateInv;

public:
	DelayEnvelope()
	{
		phase = 0;
		phaseInc = 0;
		spread = 1;
	}
	void keyResetPhase()
	{
		phase = 0;
	}
	inline float getVal()
	{
		return phase;
	}
	void setSampleRate(float val)
	{
		sampleRate = val;
		sampleRateInv = 1 / sampleRate;
	}
	inline void update()
	{
		if (phase < 1) {
			phase += phaseInc * sampleRateInv;
			if (phase > 1)
				phase = 1;
		}
	}
	void setSpread(float val)
	{
		spread = val;
	}
	void setRate(float val)
	{
		phaseInc = val;
	}
};
