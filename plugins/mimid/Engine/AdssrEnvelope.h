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
	float Value, HValue;
	float Draincount;
	float attack, hold, decay, sustain, sustainTime, release; // saved parameter values with deriverence
	float sustain_asymptote;
	bool adsrMode;
	bool linear;
	float ua, uh, ud, us, ur; // saved parameter values (not for sustain)
	float coef_atk, coef_dec, coef_sust, coef_rel;
	float coef_atk_lin, coef_hld_lin, coef_dec_lin, coef_sust_lin, coef_rel_lin;
	float dir; // decay curve direction (1 => down, -1 = up)
	enum { INI, ATK, HLD, DEC, SUS, SUST, REL, DRAIN, OFF } state, post_dec_state;
	float SampleRateKcInv;
	float uf;
	// In ADSSR mode, the asymptote is lower than the sustain level,
	// partly in order to get a trigger point at all (an exponential decay
	// theoretically never reaches its asymptote), and partly to trigger
	// the sustain phase before too much of the exponential 'tail' of the
	// decay curve has passed.
	const float sustain_delta = 0.2f;

	inline float calc_coef(float timeparam)
	{
		return SampleRateKcInv / timeparam;
	}

	inline void calc_coef_atk(float timeparam)
	{
		coef_atk = calc_coef(timeparam);
		coef_atk_lin = coef_atk * 0.7f;
	}
	inline void calc_coef_hld(float timeparam)
	{
		// Since the hold phase does not reflect a waveform
		// progression, we don't distinguish between linear and
		// exponential and do the simplest thing: handle the hold
		// phase using a linear calculation.
		coef_hld_lin = calc_coef(timeparam);
	}
	inline void calc_coef_dec(float timeparam)
	{
		coef_dec = calc_coef(timeparam);
		coef_dec_lin = coef_dec * 0.1f;
		if (!adsrMode) {
			// In ADSSR mode, compensate for the fact
			// that the sustain asymptote is lower than
			// in ADSR mode: The coefficient needs to
			// be decreased to get the curve (above the
			// asymptote) to be the same as in ADSR mode.
			coef_dec *= 1.0f - sustain;
			coef_dec /= 1.0f - sustain_asymptote;
		}
	}
	inline void calc_coef_sust(float timeparam)
	{
		coef_sust = calc_coef(timeparam);
		coef_sust_lin = coef_sust * 0.1f;
	}
	inline void calc_coef_rel(float timeparam)
	{
		coef_rel = calc_coef(timeparam);
		coef_rel_lin = coef_rel * 0.1f;
	}
	inline void calc_sustain_asymptote()
	{
		sustain_asymptote = sustain - (!adsrMode * sustain_delta);
	}
public:
	float unused1; // TODO: remove
	float unused2; // TODO: remove

	AdssrEnvelope()
	{
		uf = 1;
		Value = HValue = 0.0f;
		attack = hold = decay = sustain = sustainTime = release = 0.0001f;
		sustain_asymptote = sustain; // It is, in ADSR mode
		ua = uh = ud = us = ur = 0.0001f;
		coef_atk = coef_dec = coef_sust = coef_rel = 0;
		coef_atk_lin = coef_dec_lin = coef_sust_lin = coef_rel_lin = 0;
		post_dec_state = SUS;
		dir = 1.0f; // going down
		state = OFF;
		SampleRateKcInv = 1000.0 / 44100.0;
		adsrMode = true;
		linear = false;
	}
	void ResetEnvelopeState()
	{
		Value = HValue= 0.0f;
		state = OFF;
	}
	void setSampleRate(float sr)
	{
		SampleRateKcInv = 1.0 / (sr / 1000);
		calc_coef_atk(attack);
		calc_coef_hld(hold);
		calc_coef_dec(decay);
		calc_coef_sust(sustainTime);
		calc_coef_rel(release);
	}
	void setSpread(float spread)
	{
		uf = spread;
		setAttack(ua);
		setHold(uh);
		setDecay(ud);
		setSustainTime(us);
		setRelease(ur);
	}
	void setAdsr(bool adsr)
	{
		adsrMode = adsr;
		calc_sustain_asymptote();
		calc_coef_dec(decay);
		post_dec_state = adsrMode ? SUS : SUST;
	}
	void setLinear(bool lin)
	{
		linear = lin;
	}
	void setAttack(float atk)
	{
		ua = atk;
		attack = atk * uf;
		calc_coef_atk(attack);
	}
	void setHold(float hld)
	{
		uh = hld;
		hold = hld * uf;
		calc_coef_hld(hold);
	}
	void setDecay(float dec)
	{
		ud = dec;
		decay = dec * uf;
		calc_coef_dec(decay);
	}
	void setSustain(float sus)
	{
		sustain = sus;
		calc_sustain_asymptote();
		calc_coef_dec(decay);
		if (state == DEC || state == SUS) {
			// Chase sustain level at decay rate, if sustain
			// level changed in ADSR mode, or in ADSSR mode
			// while decay phase is still active.
			if (Value > sustain) {
				dir = 1.0f;
				state = DEC;
			} else if (Value < sustain) {
				dir = -1.0f;
				state = DEC;
			}
		}
	}
	void setSustainTime(float sust)
	{
		us = sust;
		sustainTime = sust * uf;
		calc_coef_sust(sustainTime);
	}
	void setRelease(float rel)
	{
		ur = rel;
		release = rel * uf;
		calc_coef_rel(release);
	}
	void triggerAttack()
	{
		state = INI;
		HValue = 0.0f;
		dir = 1.0f;
		//Value = Value + 0.00001f;
	}
	void triggerRelease()
	{
		if (state < DRAIN) // All except DRAIN and OFF
			state = REL;
	}
	inline bool isActive()
	{
		return state != OFF;
	}
	inline float processSample()
	{
		switch (state)
		{
		case INI:
			// Do nothing, just delay envelope for one cycle
			// This causes the output to start at 0 rather than
			// after the first increment
			state = ATK;
			break;
		case ATK:
			Value += linear ? coef_atk_lin : (1.3f - Value) * coef_atk;
			if (Value > 1.0f) {
				Value = 1.0f;
				state = HLD;
			}
			break;
		case HLD:
			HValue += coef_hld_lin;
			if (HValue < 1.0f)
				break;
			state = DEC;
			[[fallthrough]];
		case DEC:
			// Aim for sustain level
			Value -= linear ? coef_dec_lin * dir : (Value - sustain_asymptote) * coef_dec;
			// Trigger sustain phase when we transition across
			// the sustain level. We need to take into
			// consideration the case of the sustain level
			// parameter being increased while the envelope is in
			// the ADSR sustain phase, hence the decay curve can
			// actually go upwards, which we handle using the
			// dir (1 or -1) variable.
			if ((Value - sustain) * dir < 0) {
				Value = sustain;
				state = post_dec_state;
			}
			break;
		case SUS: // Used for ADSR Sustain phase
			// Leave Value as it is
			break;
		case SUST: // Use for ADSSR Sustain phase
			   // same calculations as for Release phase
			Value -= linear ? coef_sust_lin : Value * coef_sust + dc;
			if (Value < 20e-6) {
				Value = 0;
				Draincount = Samples * 8;
				state = DRAIN;
			}
			break;
		case REL: // Used for Release phase
			Value -= linear ? coef_rel_lin : Value * coef_rel + dc;
			if (Value < 20e-6) {
				Value = 0;
				Draincount = Samples * 8;
				state = DRAIN;
			}
			break;
		case DRAIN:
			if (!--Draincount)
				state = OFF;
			break;
		case OFF:
			Value = 0;
			break;
		}
		return Value;
	}
};
