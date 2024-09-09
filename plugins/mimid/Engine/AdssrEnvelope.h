/*
	==============================================================================
	This file is part of the MiMi-d synthesizer,
	originally from Obxd synthesizer.

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
#include "Voice.h"
class AdssrEnvelope
{
private:
	float Value;
	float attack, decay, sustain, sustainTime, release; // saved parameter values with deriverence
	float sustain_asymptote;
	bool adsrMode;
	bool linear;
	float ua,ud,us,ur; // saved parameter values (not for sustain)
	float coef_atk, coef_dec, coef_sust, coef_rel;
	int dir; // decay curve direction (1 => down, -1 = up)
	enum { HLD, ATK, DEC, SUS, SUST, REL, OFF } state;
	float SampleRate;
	float uf;
	// In ADSSR mode, the asymptote is lower than the sustain level,
	// partly in order to get a trigger point at all (an exponential decay
	// theoretically never reaches its asymptote), and partly to trigger
	// the sustain phase before too much of the exponential 'tail' of the
	// decay curve has passed.
	const float sustain_delta = 0.2f;

	inline float calc_coef_atk(float timeparam)
	{
		return 1.0f / (SampleRate * (timeparam)/1000.0) * (linear ? 0.7f : 1.0f);
	}
	inline float calc_coef_dec(float timeparam)
	{
		float coef = 1.0f / (SampleRate * (timeparam) / 1000.0f);
		if (linear)
			coef *= 0.1f;
		else if (!adsrMode) {
			// In ADSSR mode, compensate for the fact
			// that the sustain asymptote is lower than
			// in ADSR mode: The coefficient needs to
			// be decreased to get the curve (above the
			// asymptote) to be the same as in ADSR mode.
			coef *= 1.0f - sustain;
			coef /= 1.0f - sustain_asymptote;
		}
		return coef;
	}
	inline float calc_coef_sust(float timeparam)
	{
		return 1.0f / (SampleRate * (timeparam) / 1000) * (linear ? 0.10 : 1);
	}
	inline float calc_coef_rel(float timeparam)
	{
		return 1.0f / (SampleRate * (timeparam) / 1000.0f) * (linear ? 0.1f : 1.0f);
	}
	inline float calc_sustain_asymptote()
	{
		return sustain - (adsrMode ? 0 : sustain_delta);
	}
public:
	float unused1; // TODO: remove
	float unused2; // TODO: remove

	AdssrEnvelope()
	{
		uf = 1;
		Value = 0.0f;
		attack=decay=sustain=sustainTime=release=0.0001f;
		sustain_asymptote = sustain; // It is, in ADSR mode
		ua=ud=us=ur=0.0001;
		coef_atk = coef_dec = coef_sust = coef_rel = 0;
		dir = 1; // going down
		state = OFF;
		SampleRate = 44100.0f;
		adsrMode = true;
		linear = false;
	}
	void ResetEnvelopeState()
	{
		Value = 0.0f;
		state = OFF;
	}
	void setSampleRate(float sr)
	{
		SampleRate = sr;
	}
	void setSpread(float spread)
	{
		uf = spread;
		setAttack(ua);
		setDecay(ud);
		setSustainTime(us);
		setRelease(ur);
	}
	void setAdsr(bool adsr)
	{
		adsrMode = adsr;
		if (state == DEC || state == SUS) {
			sustain_asymptote = calc_sustain_asymptote();
			coef_dec = calc_coef_dec(decay);
		}
	}
	void setLinear(bool lin)
	{
		linear = lin;
		if (state == DEC || state == SUS)
			coef_dec = calc_coef_dec(decay);
		else if (state == SUST)
			coef_sust = calc_coef_sust(sustainTime);
		else if (state == REL)
			coef_rel = calc_coef_rel(release);
		else if (state == ATK)
			coef_atk = calc_coef_atk(attack);
	}
	void setAttack(float atk)
	{
		ua = atk;
		attack = atk*uf;
		if (state == ATK)
			coef_atk = calc_coef_atk(attack);
	}
	void setDecay(float dec)
	{
		ud = dec;
		decay = dec*uf;
		if (state == DEC || state == SUS)
			coef_dec = calc_coef_dec(decay);
	}
	void setSustain(float sus)
	{
		sustain = sus;
		if (state == DEC || state == SUS) {
			sustain_asymptote = calc_sustain_asymptote();
			// Chase sustain level at decay rate, if sustain
			// level changed in ADSR mode
			if (Value > sustain) {
				dir = 1;
				state = DEC;
			} else if (Value < sustain) {
				dir = -1;
				state = DEC;
			}
		}
	}
	void setSustainTime(float sust)
	{
		us = sust;
		sustainTime = sust*uf;
		if (state == SUST)
			coef_sust = calc_coef_sust(sustainTime);
	}
	void setRelease(float rel)
	{
		ur = rel;
		release = rel*uf;
		if (state == REL)
			coef_rel = calc_coef_rel(release);
	}
	void triggerAttack()
	{
		state = HLD;
		//Value = Value +0.00001f;
		coef_atk = calc_coef_atk(attack);
	}
	void triggerRelease()
	{
		if (state != OFF) {
			coef_rel = calc_coef_rel(release);
			state = REL;
		}
	}
	inline bool isActive()
	{
		return state != OFF;
	}
	inline float processSample()
	{
		switch (state)
		{
		case HLD:
			// Do nothing, just delay envelope for one cycle
			// This causes the output to start at 0 rather than
			// after the first increment
			state = ATK;
			break;
		case ATK:
			Value += linear ? coef_atk : (1.3f - Value) * coef_atk;
			if (Value > 1.0f) {
				Value = 1.0f;
				state = DEC;
				sustain_asymptote = calc_sustain_asymptote();
				coef_dec = calc_coef_dec(decay);
				dir = 1;
			}
			break;
		case DEC:
			// Aim for sustain level
			Value -= linear ? coef_dec * dir : (Value - sustain_asymptote) * coef_dec;
			// Trigger sustain phase when we transition across
			// the sustain level. We need to take into
			// consideration the case of the sustain level
			// parameter being increased while the envelope is in
			// the ADSR sustain phase, hence the decay curve can
			// actually go upwards, which we handle using the
			// dir (1 or -1) variable.
			if ((Value - sustain) * dir < 0) {
				Value = sustain;
				if (adsrMode)
					state = SUS;
				else {
					state = SUST;
					coef_sust = calc_coef_sust(sustainTime);
				}
			}
			break;
		case SUS: // Used for ADSR Sustain phase
			// Leave Value as it is
			break;
		case SUST: // Use for ADSSR Sustain phase
			   // same calculations as for Release phase
			Value -= linear ? coef_sust : Value * coef_sust + dc;
			if (Value < 20e-6) {
				Value = 0;
				state = OFF;
			}
			break;
		case REL: // Used for Release phase
			Value -= linear ? coef_rel : Value * coef_rel + dc;
			if (Value < 20e-6) {
				Value = 0;
				state = OFF;
			}
			break;
		case OFF:
			Value = 0;
			break;
		}
		return Value;
	}
};
