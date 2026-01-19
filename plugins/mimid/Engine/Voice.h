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

#include "Random.h"
#include "Oscillators.h"
#include "AdssrEnvelope.h"
#include "Lfo.h"
#include "Filter.h"
#include "Decimator.h"
#include "SquareDist.h"
#include "FastExp.h"
#include "ParamSmoother.h"

float dummyfloat;

class ModRoute
{
private:
	float *dest1, *dest2;
	float scale;

public:
	ModRoute()
	{
		// Set a dummy route
		dest1 = &dummyfloat;
		dest2 = NULL;
		scale = 0;
	}

	inline void modulate(float source)
	{
		float amount = source * scale;
		*dest1 += amount;
		if (dest2)
			*dest2 += amount;
	}

	void setRoute(float *destination1, float *destination2, float scalefactor)
	{
		dest1 = destination1;
		dest2 = destination2;
		scale = scalefactor;
	}
};

class Voice
{
private:
	float audioRate;
	float audioRateInv;
	float modRate;
	float modRateInv;
	float maxfiltercutoff;
	float velocityValue;

	// State variables for various single pole filters
	float oschpfst; // 12 Hz oscillator HPF
	float prtst; // portamento
	float hpfst; // HPF between filter and VCA

	// offset to get apparent zero cutoff frequency shift with oscmod
	const float oscmod_offset = 0.20;
	// maximum peak of oscmod waveform w/ offset applied
	const float oscmod_maxpeak = 0.5 - oscmod_offset;
	const float oscmod_maxpeak_inv = oscmod_maxpeak ? 1/oscmod_maxpeak : 0;

	float zero = 0;

public:
	AdssrEnvelope env;
	AdssrEnvelope fenv;
	Lfo lfo1;
	Lfo lfo2;
	Lfo lfo3;
	OscillatorParams oscparams;
	OscillatorModulation oscmodulation;
	Oscillators osc;
	Filter flt;
	SquareDist sqdist;

	ModRoute lfo1route;
	ModRoute lfo2route;
	ModRoute lfo3route;
	ModRoute pwroute;
	ModRoute modroute;
	ModRoute atroute;

	SRandom ng;

	ParamSmoother afterTouchSmoother;

	bool sustainHold;

	float vamp, vflt;
	float velscale;

	float envVal;
	float cutoff;
	float filteroct;
	float filtertune;
	float fenvamt;
	float res;

	float EnvSpread;
	float FenvSpread;

	float Lfo1Spread;
	float Lfo2Spread;
	float Lfo3Spread;

	float FltSpread; // Random amount calculated at start
	float FltSpreadAmt; // Calculated value depending on parameter

	float PortaSpread; // Random amount calculated at start
	float PortaSpreadAmt; // Calculated value depending on parameter

	float levelSpread; // Random amount calculated at start
	float levelSpreadAmt; // Calculated value depending on parameter

	float osc2FltMod;

	float hpffreq, hpfcutoff;

	int midiIndx;

	bool Active; // = Gate, set on at Note On, off at Note Off
	bool shouldProcess; // Lenv is not off, i.e. DSP should be run
	Voice *buddy;

	bool oscKeySync;
	bool envRst;

	float fltKF;

	float porta, portaSaved;
	bool portaEnable;

	float pitchWheel, pitchWheelAmt;

	float lfo1amt, lfo2amt, lfo3amt;
	float lfo1contramt, lfo2contramt, lfo3contramt;
	float *lfo1controller, *lfo2controller, *lfo3controller;

	// Modwheel and aftertouch values
	float modw, aftert;
	float modWheelAmt, afterTouchAmt;

	bool invertFenv;

	// Modulatable entitites
	float osc2FltModCalc;
	float cutoffnote;
	float rescalc;

	DelayLineRampable<Samples*2> lenvd;
	DelayLine<Samples*2> cutoffd, resd, bmodd;

	bool oscmodEnable; // Oscillator modulation output enabled

	bool expvca;

	int voiceNumber; // Handy to have in the voice itself

	float unused1, unused2; // TODO: remove

	Voice(): osc(oscparams, oscmodulation), afterTouchSmoother()
	{
		maxfiltercutoff = 22000.0f;
		invertFenv = false;
		ng = SRandom(SRandom::globalRandom().nextInt32());
		sustainHold = false;
		shouldProcess = false;
		buddy = NULL;
		vamp = vflt = 0;
		velscale = 1;
		velocityValue = 0;
		oscKeySync = false;
		envRst = false;
		hpffreq = 4;
		hpfcutoff = 0;
		osc2FltMod = 0;
		pitchWheel = pitchWheelAmt = 0;
		PortaSpreadAmt = 1;
		FltSpreadAmt = 0;
		levelSpreadAmt = 1;
		portaSaved = porta = 0;
		portaEnable = false;
		oschpfst = hpfst = prtst = 0;
		fltKF = false;
		cutoff = 0;
		filteroct = 0;
		filtertune = 0;
		fenvamt = 0;
		res = 0;
		Active = false;
		midiIndx = 30;
		levelSpread = SRandom::globalRandom().nextFloat()-0.5;
		EnvSpread = SRandom::globalRandom().nextFloat()-0.5;
		FenvSpread = SRandom::globalRandom().nextFloat()-0.5;
		Lfo1Spread = SRandom::globalRandom().nextFloat()-0.5;
		Lfo2Spread = SRandom::globalRandom().nextFloat()-0.5;
		Lfo3Spread = SRandom::globalRandom().nextFloat()-0.5;
		FltSpread = SRandom::globalRandom().nextFloat()-0.5;
		PortaSpread = SRandom::globalRandom().nextFloat()-0.5;
		oscmodEnable = false;
		expvca = false;
		voiceNumber = 0; // Until someone else says something else
		unused1 = unused2 = 0; // TODO: Remove
		cutoffnote = 0;
		rescalc = 0;
		osc2FltModCalc = 0;
		lfo1controller = lfo2controller = lfo3controller = &zero;
	}
	~Voice()
	{
	}
	inline void processModulation()
	{
		// LFOs
		float lfo1In = lfo1.getVal();
		float lfo2In = lfo2.getVal();
		float lfo3In = lfo3.getVal();

		// Multiplying modamt with (1-lfoamt) scales
		// the modulation so that the total value never goes above 1.0
		// no matter what combination of amount and modwheel/aftertouch
		// is dialed in.
		float lfo1totalamt = lfo1amt +
			*lfo1controller * lfo1contramt * (1 - lfo1amt);
		float lfo2totalamt = lfo2amt +
			*lfo2controller * lfo2contramt * (1 - lfo2amt);
		float lfo3totalamt = lfo3amt +
			*lfo3controller * lfo3contramt * (1 - lfo3amt);

		// Both envelopes and filter cv need a delay equal to osc internal delay
		// Bipolar filter envelope
		float envm = fenv.processSample() * (1 - (1-2*velocityValue)*vflt);
		envm = 2 * envm - 1; // make bipolar
		if (invertFenv)
			envm = -envm;

		// Loudness envelope, with delay (same reason as for cutoff)
		envVal = lenvd.feedReturn(env.processSample() * (1 - (1-velocityValue)*vamp));

		// PW modulation
		osc.oscmodulation.sh1 = 0;
		osc.oscmodulation.sh2 = 0;

		// Pitch modulation
		osc.oscmodulation.pto1 = 0;
		osc.oscmodulation.pto2 = 0;

		// Midi note 93 is A6 (1760 Hz), so ptNote == 0 => 1760 Hz
		// Pitch calc base frequency is 440 Hz, but the default
		// osc pitch is 24 (semitones), resulting in
		// 440 Hz + 2 octaves = 440 * 2 * 2 = 1760 Hz.
		// (Default osc tuning at midi 60 is middle C = C4 = 261.63 Hz)
		// Portamento on osc input voltage using LPF
		float ptNote = tptlpupw(prtst, midiIndx-93, porta * PortaSpreadAmt, modRateInv);
		osc.notePlaying = ptNote;

		// Filter cutoff and resonance
		// ptNote+54 => Eb2 = 77.78 Hz is base note for filter tracking
		cutoffnote =
			cutoff +
			FltSpreadAmt +
                        fenvamt * envm +
			-54 + (fltKF * (ptNote + filteroct + filtertune + 54));

		rescalc = res;

		// Bmod
		osc2FltModCalc = osc2FltMod;

		// Here we provide modulation, so all modulation sources and
		// destination variables need to be initialized at this point
		// for the mod routings to take effect.
		lfo1route.modulate(lfo1In * lfo1totalamt);
		lfo2route.modulate(lfo2In * lfo2totalamt);
		lfo3route.modulate(lfo3In * lfo3totalamt);
		pwroute.modulate(pitchWheel * pitchWheelAmt);
		modroute.modulate(modw * modWheelAmt);
		atroute.modulate(aftert * afterTouchAmt);

		// Filter audio is delayed because it runs on the oscillator
		// output, so delay control signals to filter as well.
		// VCA has no modulation but loudness envelope is delayed too
		// because it comes after the filter
		cutoffnote = cutoffd.feedReturn(cutoffnote);
		osc2FltModCalc = bmodd.feedReturn(osc2FltModCalc);
		rescalc = resd.feedReturn(rescalc);

		// Cap resonance at 0 and +1 to avoid nasty artefacts
		rescalc = limitf(rescalc, 0.0f, 1.0f);

		// Alias limiting for oscillator filter modulation:
		// When the positive peak of the mod signal would cause
		// the filter frequency to go above ~22 kHz, scale down
		// the mod amount accordingly.
		// This gives a seamless transition from the mod value
		// set by osc2FltMod down to zero as the filter
		// frequency is increased. The effectiveness of this
		// mod routing is rather diminished at high filter
		// frequency settings anyway.
		// Note that we only limit the modulation amount, the
		// filter frequency without modulation is not limited
		// at this time.
		static const float maxfltfreq = 22000;
		static const float maxallowednote = getNote(maxfltfreq);
		// maxcutoff = cutoff freq at +ve peak of mod wave
		// (without considering oscmod_offset, which gives
		// us a bit of extra margin, as the final offset is
		// in fact negative).
		if (!oscmodEnable || cutoffnote > maxallowednote)
			// disabled or outside range; disable modulation
			osc2FltModCalc = 0;
		else {
			// TODO: Should osc2FltMod be osc2FltModCalc?
			// On the one hand, it makes sense to consider the
			// modulation excursion of osc2FltMod, on the other
			// hand it makes sense to have a stable maxcutoff.
			float maxcutoff = cutoffnote + oscmod_maxpeak * osc2FltMod;
			if (maxcutoff > maxallowednote)
			// limit osc2FltMod to keep under max allowed.
			// note: divide by peak of mod signal.
				osc2FltModCalc = (maxallowednote - cutoffnote) *
						  oscmod_maxpeak_inv;
		}
		// Calculate osc 2 waveshape parameters
		switch (osc.oscparams.osc2Wave) {
		case 2: // Pulse
			osc.pw2calc = limitf((osc.oscparams.osc2sh + osc.oscmodulation.sh2) * 0.5f + 0.5f, 0.0f, 1.0f);
			break;
		case 3: // Triangle / Trapezoid
			osc.symmetry2 = (osc.oscparams.osc2sh + osc.oscmodulation.sh2) * 0.5f + 0.5f;
			break;
		case 1:	// Saw / VariSaw
			osc.sgradient2 = superfast_exp2f_shape(osc.oscparams.osc2sh + osc.oscmodulation.sh2);
			break;
		case 0: // Off
		default:
			break;
		}

		// Calculate osc 1 waveshape parameters
		switch (osc.oscparams.osc1Wave) {
		case 2: // Pulse
			osc.pw1calc = limitf((osc.oscparams.osc1sh + osc.oscmodulation.sh1) * 0.5f + 0.5f, 0.0f, 1.0f);
			break;
		case 3: // Triangle / Trapezoid
			osc.symmetry1 = (osc.oscparams.osc1sh + osc.oscmodulation.sh1) * 0.5f + 0.5f;
			break;
		case 1:	// Saw / VariSaw
			osc.sgradient1 = superfast_exp2f_shape(osc.oscparams.osc1sh + osc.oscmodulation.sh1);
			break;
		case 0: // Off
		default:
			break;
		}
	}
	inline float processAudioSample()
	{
		float oscps, oscmod;

		// Audio sample generation

		// Oscillators
		osc.ProcessSample(oscps, oscmod);

		oscps *= levelSpreadAmt;

		// HPF on oscillator output to get rid of any DC,
		// simulating a fairly large coupling capacitor.
		// TODO: filter oscmod as well to reduce aliasing?
		oscps = oscps - tptlpupw(oschpfst, oscps, 12, audioRateInv);

		// Filter exp cutoff calculation
		// Needs to be done after we've gotten oscmod
		//
		float cutoffcalc = minf(
			getPitch(cutoffnote +
				 (oscmod-oscmod_offset) * osc2FltModCalc)
			// noisy filter cutoff
			+ (ng.nextFloat()-0.5f)*3.5f, maxfiltercutoff);

		float x1 = flt.Apply4Pole(oscps, cutoffcalc, rescalc);

		// HPF
		x1 -= tptpc(hpfst, x1, hpfcutoff);

		// Distortion/overdrive
		x1 = sqdist.Apply(x1);

		// VCA
		if (expvca) {
			// Approximate exponential curve with x**5:
			// - Faster to calculate yet reasonable approximation
			// - Actually goes down to zero when envelope goes t0 0
			// Empirically, MiMi-a exponential VCA would be:
			// envVal = expf(7.5*(envVal-1)); // 1..0 -> 0..-65 dB
			float envValSquared = envVal * envVal;
			envVal *= envValSquared * envValSquared;
		}
		x1 *= envVal;
		return x1;
	}
	void setHPFfreq(float val)
	{
		hpffreq = val;
		hpfcutoff = tanf(hpffreq * audioRateInv * pi);
	}
	void setEnvSpreadAmt(float d)
	{
		// Derivation: start with 2 ** (rndval * amt), which
		// gives a range of 2**(-0.5) .. 2**(0.5) = 0.707 .. 1.41,
		// which gives us values centered around 1 when applied
		// multiplicatively as is done in the envelope.
		// Replacing 2** with exp() gives us exp(rndval * amt * ln(a)),
		// where a = maxval**2, i.e. exp(rndval * amt * ln(2)).
		// Consequently, we have exp(rndval * amt * ln(maxval ** 2)) =
		// exp(rndval * amt * 2 * ln(maxval)).
		// If we then set maxval to be the more even 1.5, we get
		// the final exp(rndval * amt * 2 * ln(1.5)) .
		// The total range will then be 0.67 .. 1.5 .
		env.setSpread(expf(EnvSpread*d * 2 * logf(1.5)));
		fenv.setSpread(expf(EnvSpread*d * 2 * logf(1.5)));
	}
	void setLfoSpreadAmt(float d)
	{
		lfo1.setSpread(expf(Lfo1Spread*d * 2 * logf(1.5)));
		lfo2.setSpread(expf(Lfo2Spread*d * 2 * logf(1.5)));
		lfo3.setSpread(expf(Lfo3Spread*d * 2 * logf(1.5)));
	}
	void setModRoute(int param, ModRoute &route)
	{
		// off, osc1, osc1+2, osc2, sh1, sh1+2, sh2, filt, res, bmod
		// 0    1     2       3     4    5      6    7     8    9
		switch (param) {
			case 0: route.setRoute(&osc.oscmodulation.pto1, NULL, 0.0f);
				break;
			case 1: route.setRoute(&osc.oscmodulation.pto1, NULL, 12.0f);
				break;
			case 2: route.setRoute(&osc.oscmodulation.pto1, &osc.oscmodulation.pto2, 12.0f);
				break;
			case 3: route.setRoute(&osc.oscmodulation.pto2, NULL, 12.0f);
				break;
			case 4: route.setRoute(&osc.oscmodulation.sh1, NULL, 1.0f);
				break;
			case 5: route.setRoute(&osc.oscmodulation.sh1, &osc.oscmodulation.sh2, 1.0f);
				break;
			case 6: route.setRoute(&osc.oscmodulation.sh2, NULL, 1.0f);
				break;
			case 7: route.setRoute(&cutoffnote, NULL, 60.0f);
				break;
			case 8: route.setRoute(&rescalc, NULL, 1.0f);
				break;
			case 9: route.setRoute(&osc2FltModCalc, NULL, 100.0f);
				break;
		}
	}
	void setMod1Route(int param)
	{
		setModRoute(param, lfo1route);
	}
	void setMod2Route(int param)
	{
		setModRoute(param, lfo2route);
	}
	void setMod3Route(int param)
	{
		setModRoute(param, lfo3route);
	}
	void setModWheelRoute(int param)
	{
		setModRoute(param, modroute);
	}
	void setAfterTouchRoute(int param)
	{
		setModRoute(param, atroute);
	}
	void setPwRoute(ModRoute &route, int param)
	{
		// OFF - OSC1 - OSC1+2
		// 0     1      2
		switch (param) {
			case 0: route.setRoute(&osc.oscmodulation.pto1, NULL, 0.0f);

				break;
			case 1: route.setRoute(&osc.oscmodulation.pto1, NULL, 1.0f);
				break;
			case 2: route.setRoute(&osc.oscmodulation.pto1, &osc.oscmodulation.pto2, 1.0f);
				break;
		}
	}
	void setModController(float **controller, int param)
	{
		// off - modwheel - aftertouch - vel
		switch (param) {
			case 0: *controller = &zero; break;
			case 1: *controller = &modw; break;
			case 2: *controller = &aftert; break;
			case 3: *controller = &velocityValue; break;
		}
	}
	void setHQ(bool hq)
	{
		if (hq)
			osc.setDecimation();
		else
			osc.removeDecimation();
	}
	void setPorta(float newPorta)
	{
		portaSaved = newPorta;
		if (portaEnable)
			porta = portaSaved;
	}
	void setSampleRate(float sr, int oversamplingRatio, int modulationRatio)
	{
		modulationRatio += (modulationRatio == 0); // avoid div by 0
		modRate = sr / modulationRatio;
		modRateInv = 1 / modRate;
		audioRate = sr * oversamplingRatio;
		audioRateInv = 1 / audioRate;

		flt.setSampleRate(audioRate);
		osc.setSampleRate(audioRate);
		env.setSampleRate(modRate);
		fenv.setSampleRate(modRate);
		lfo1.setSampleRate(modRate);
		lfo2.setSampleRate(modRate);
		lfo3.setSampleRate(modRate);
		afterTouchSmoother.setSampleRate(modRate);
		hpfcutoff = tanf(hpffreq * audioRateInv * pi);
		// Limit filter freq to nyquist frequency minus a small
		// margin (for numerical stability reasons), or 22 kHz,
		// whichever is smaller.
		// There's no reason to go higher (the filter frequency
		// modulation amount is capped when the cutff reaches
		// this frequency , and at around 28 kHz or so there is
		// a small range where there are 'birdies' evident in the
		// output for some reason).
		maxfiltercutoff = minf(audioRate*0.5f - 120.0f, 22000.0f);

		// Delay lines compensate for the delay added in
		// oscillator class, so we need to adjust the length
		// depending on the oversampling ratio so the delay
		// lines have the same length in units of time.
		int delayLineLength = 2 * Samples / oversamplingRatio  / modulationRatio;
		// If length is 1 we get no delay at all, so minimize at 2
		if (delayLineLength < 2) delayLineLength = 2;
		lenvd.setLength(delayLineLength);
		cutoffd.setLength(delayLineLength);
		resd.setLength(delayLineLength);
		bmodd.setLength(delayLineLength);
	}
	void checkAdssrState()
	{
		shouldProcess = env.isActive();
	}
	void ResetEnvelopes()
	{
		env.ResetEnvelopeState();
		fenv.ResetEnvelopeState();
	}
	void NoteOn(int mididx, float velocity, bool multiTrig, bool doPorta = true)
	{
		if (!shouldProcess)
		{
			// When processing is paused we need to clear delay
			// lines and envelopes.
			// Not doing this will cause clicks or glitches.
			lenvd.fillZeroes();
			cutoffd.fillZeroes();
			bmodd.fillZeroes();
			resd.fillZeroes();
			ResetEnvelopes();
		}
		shouldProcess = true;
		if (velocity != -0.5)
			// Scale velocity according to velscale [ 8..1..1/8 ]
			// range is same (0..1 -> 0..1), but scale changes
			velocityValue = powf(velocity, velscale);
		midiIndx = mididx;
		if (!Active || multiTrig) {
			if (envRst) {
				ResetEnvelopes();
				// Ramp down whatever is in the loudness env
				// delay line to zero to minimize clicking
				// when envelopes are reset.
				lenvd.decayLine();
			}
			env.triggerAttack();
			fenv.triggerAttack();
			lfo1.keyResetPhase();
			lfo2.keyResetPhase();
			lfo3.keyResetPhase();
		}
		if (oscKeySync)
			osc.keyReset = true;
		Active = true;
		portaEnable = doPorta;
		porta = portaEnable ? portaSaved : 250;
	}
	void NoteOff()
	{
		if (!sustainHold) {
			env.triggerRelease();
			fenv.triggerRelease();
		}
		Active = false;
	}
	void NoteOffImmediately()
	{
		ResetEnvelopes();
		// Ramp down whatever is in the loudness env
		// delay line to zero to minimize clicking
		// when envelopes are reset.
		lenvd.decayLine();
		Active = false;
	}
	void sustOn()
	{
		sustainHold = true;
	}
	void sustOff()
	{
		sustainHold = false;
		if (!Active)
		{
			env.triggerRelease();
			fenv.triggerRelease();
		}

	}
};
