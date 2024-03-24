/*
	==============================================================================
	This file is part of the MiMi-d synthesizer,
	originally from Obxd synthesizer.

	Copyright © 2013-2014 Filatov Vadim
	Copyright 2023 Ricard Wanderlof

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
#include "Oscillators.h"
#include "AdssrEnvelope.h"
#include "Lfo.h"
#include "Filter.h"
#include "Decimator.h"
#include "SquareDist.h"

class Voice
{
private:
	float SampleRate;
	float sampleRateInv;
	float Volume;
	float port;
	float velocityValue;

	float d1,d2;
	float c1,c2;
	float shpf;

	// offset to get apparent zero cutoff frequency shift with oscmod
	const float oscmod_offset = 0.20;
	// maximum peak of oscmod waveform w/ offset applied
	const float oscmod_maxpeak = 0.5 - oscmod_offset;
	const float oscmod_maxpeak_inv = oscmod_maxpeak ? 1/oscmod_maxpeak:0;

	//JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Voice)
public:
	bool sustainHold;
	//bool resetAdssrsOnAttack;

	AdssrEnvelope env;
	AdssrEnvelope fenv;
	Lfo lfo1;
	Lfo lfo2;
	Oscillators osc;
	Filter flt;
	SquareDist sqdist;

	Random ng;

	float vamp,vflt;
	float velscale;

	float cutoff;
	float fenvamt;
	float res;

	float EnvSpread;
	float FenvSpread;

	float Lfo1Spread;
	float Lfo2Spread;

	float FltSpread;
	float FltSpreadAmt;

	float PortaSpread;
	float PortaSpreadAmt;

	float levelSpread;
	float levelSpreadAmt;

	float brightCoef;
	float osc2FltMod;

	float hpffreq, hpfcutoff;

	int midiIndx;

	bool Active;
	bool shouldProcessed;

	bool oscKeySync;
	bool envRst;

	float fltKF;

	float porta, portaSaved;
	bool portaEnable;
	float prtst;

	float cutoffwas,envelopewas;

	bool pitchWheelOsc1, pitchWheelOsc2;
	float pitchWheel,pitchWheelAmt;

	float lfo1amt, lfo2amt;
	float lfo1modamt, lfo2modamt;
	float modw, aftert;
	bool lfo1o1,lfo1o2,lfo1pw1,lfo1pw2,lfo1f,lfo1bmod,lfo1res;
	bool lfo2o1,lfo2o2,lfo2pw1,lfo2pw2,lfo2f,lfo2bmod,lfo2res;

	bool selfOscPush;

	bool invertFenv;
	bool lfo1modw, lfo1after, lfo1vel;
	bool lfo2modw, lfo2after, lfo2vel;

	DelayLineRampable<Samples*2> lenvd,fenvd;
	DelayLine<Samples*2> lfo1d,lfo2d;

	float oscpsw;
	float briHold;

	bool oscmodEnable; // Oscillator modulation output enabled

	bool expvca;

	int voiceNumber; // Handy to have in the voice itself

	float unused1, unused2; // TODO: remove

	Voice()
	{
		selfOscPush = false;
		invertFenv = false;
		ng = Random(Random::getSystemRandom().nextInt64());
		sustainHold = false;
		shouldProcessed = false;
		vamp=vflt=0;
		velscale=1;
		velocityValue=0;
		oscKeySync = false;
		envRst = false;
		brightCoef =briHold= 1;
		hpffreq=4;
		hpfcutoff=0;
		osc2FltMod = 0;
		oscpsw = 0;
		cutoffwas = envelopewas=0;
		c1=c2=d1=d2=shpf=0;
		pitchWheel=pitchWheelAmt=0;
		PortaSpreadAmt=0;
		FltSpreadAmt=0;
		levelSpreadAmt=0;
		portaSaved=porta =0;
		portaEnable=false;
		prtst=0;
		fltKF= false;
		cutoff=0;
		fenvamt = 0;
		res=0;
		Active = false;
		midiIndx = 30;
		levelSpread = Random::getSystemRandom().nextFloat()-0.5;
		EnvSpread = Random::getSystemRandom().nextFloat()-0.5;
		FenvSpread = Random::getSystemRandom().nextFloat()-0.5;
		Lfo1Spread = Random::getSystemRandom().nextFloat()-0.5;
		Lfo2Spread = Random::getSystemRandom().nextFloat()-0.5;
		FltSpread = Random::getSystemRandom().nextFloat()-0.5;
		PortaSpread =Random::getSystemRandom().nextFloat()-0.5;
	//	lenvd=new DelayLine(Samples*2);
	//	fenvd=new DelayLine(Samples*2);
		oscmodEnable = false;
		expvca = false;
		lfo2.waveForm = 1; // Triangle
		voiceNumber = 0; // Until someone else says something else
		unused1=unused2=0; // TODO: Remove
	}
	~Voice()
	{
	//	delete lenvd;
	//	delete fenvd;
	}
	inline float ProcessSample()
	{
		float oscps, oscmod;

		//bipolar filter envelope undelayed
		float envm = 2 * (fenv.processSample() * (1 - (1-2*velocityValue)*vflt)) - 1;
		if(invertFenv)
			envm = -envm;

		//filter envelope undelayed

		float lfo1In = lfo1.getVal();
		float lfo2In = lfo2.getVal();

		//Multiplying modamt with (1-lfoamt) scales
		//the modulation so that the total value never goes above 1.0
		//no matter what combination of amount and modwheel/aftertouch
		//is dialed in.
		float lfo1totalamt = lfo1amt +
				     ((lfo1modw ? modw : 0) +
				      (lfo1after ? aftert : 0) +
				      (lfo1vel ? velocityValue : 0)) *
					lfo1modamt * (1 - lfo1amt);
		float lfo2totalamt = lfo2amt +
				     ((lfo2modw ? modw : 0) +
				      (lfo2after ? aftert : 0) +
				      (lfo2vel ? velocityValue : 0)) *
					 lfo2modamt * (1 - lfo2amt);

		float lfo1mod = lfo1In * lfo1totalamt;
		float lfo2mod = lfo2In * lfo2totalamt;

		//portamento on osc input voltage
		//implements rc circuit
		// Midi note 81 is A5 (880 Hz), so ptNote == 0 => 880 Hz
		float ptNote  =tptlpupw(prtst, midiIndx-81, porta * (1+PortaSpread*PortaSpreadAmt),sampleRateInv);
		osc.notePlaying = ptNote;
		//both envelopes and filter cv need a delay equal to osc internal delay
		float lfo1Delayed = lfo1d.feedReturn(lfo1mod);
		float lfo2Delayed = lfo2d.feedReturn(lfo2mod);

		//PW modulation
		osc.pw1 = (lfo1pw1?lfo1mod:0) + (lfo2pw1?lfo2mod:0);
		osc.pw2 = (lfo1pw2?lfo1mod:0) + (lfo2pw2?lfo2mod:0);

		//Pitch modulation
		osc.pto1 = (pitchWheelOsc1?(pitchWheel*pitchWheelAmt):0) + (lfo1o1?lfo1mod*12:0) + (lfo2o1?lfo2mod*12:0);
		osc.pto2 = (pitchWheelOsc2?(pitchWheel*pitchWheelAmt):0) + (lfo1o2?lfo1mod*12:0) + (lfo2o2?lfo2mod*12:0);

		//variable sort magic - upsample trick
		float envVal = lenvd.feedReturn(env.processSample() * (1 - (1-velocityValue)*vamp));

		osc.ProcessSample(oscps, oscmod);

		oscps *= 1 - levelSpreadAmt*levelSpread;
		oscps = oscps - tptlpupw(c1,oscps,12,sampleRateInv);

		//filter exp cutoff calculation
		//needs to be done after we've gotten oscmod
		// ptNote+40 => F2 = 87.31 Hz is base note for filter tracking
		float cutoffnote =
			(lfo1f?(lfo1Delayed*60):0)+
			(lfo2f?(lfo2Delayed*60):0)+
			cutoff+
			FltSpread*FltSpreadAmt+
			fenvamt*fenvd.feedReturn(envm)+
			-45 + (fltKF*(ptNote+40));
		if (oscmodEnable) {
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
			float maxcutoff = cutoffnote + oscmod_maxpeak * osc2FltMod;
			float osc2FltModTmp = osc2FltMod + (lfo1bmod?(lfo1Delayed*100):0) + (lfo2bmod?(lfo2Delayed*100):0);
			if (cutoffnote > maxallowednote)
				// outside range; disable modulation
				osc2FltModTmp = 0;
			else if (maxcutoff > maxallowednote)
				// limit osc2FltMod to keep under max allowed.
				// note: divide by peak of mod signal.
				osc2FltModTmp = (maxallowednote - cutoffnote) *
						oscmod_maxpeak_inv;
			cutoffnote += (oscmod-oscmod_offset) * osc2FltModTmp;
		}
		float cutoffcalc = jmin(
			getPitch(cutoffnote)
			//noisy filter cutoff
			+(ng.nextFloat()-0.5f)*3.5f
			, (flt.SampleRate*0.5f-120.0f));//for numerical stability purposes
		//limit our max cutoff on self osc to prevent alising
		//TODO: higher limit when oversample enabled?
		if(selfOscPush)
			cutoffcalc = jmin(cutoffcalc,19000.0f);

		float x1 = oscps;
		//TODO: filter oscmod as well to reduce aliasing?
		// Cap resonance at 0 and +1 to avoid nasty artefacts
		float rescalc = jmin(jmax(res + (lfo1res?lfo1Delayed:0) + (lfo2res?lfo2Delayed:0), 0.0f), 1.0f);

		x1 = flt.Apply4Pole(x1, cutoffcalc, rescalc);

		// HPF
		x1 -= tptpc(shpf, x1, hpfcutoff);

		// Distortion/overdrive
		x1 = sqdist.Apply(x1);

		// VCA
		if (expvca) {
			// Approximate exponential curve with x**5:
			// - Faster to calculate yet reasonable approximation
			// - Actually goes down to zero when envelope goes t0 0
			// Empirically, MiMi-a exponential VCA would be:
			// envVal = expf(7.5*(envVal-1)); // 1..0 -> 0..-65 dB
			envVal *= envVal * envVal * envVal * envVal;
		}
		x1 *= envVal;
		return x1;
	}
	void setBrightness(float val)
	{
		briHold = val;
		brightCoef = tan(jmin(val,flt.SampleRate*0.5f-10)* pi*flt.sampleRateInv);

	}
	void setHPFfreq(float val)
	{
		hpffreq = val;
		hpfcutoff = tan(hpffreq * sampleRateInv * pi);
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
	}

	void setHQ(bool hq)
	{
		if(hq)
		{
			osc.setDecimation();
		}
		else
		{
			osc.removeDecimation();
		}
	}
	void setPorta(float newPorta)
	{
		portaSaved = newPorta;
		if (portaEnable)
			porta = portaSaved;
	}
	void setSampleRate(float sr)
	{
		flt.setSampleRate(sr);
		osc.setSampleRate(sr);
		env.setSampleRate(sr);
		fenv.setSampleRate(sr);
		lfo1.setSampleRate(sr);
		lfo2.setSampleRate(sr);
		SampleRate = sr;
		sampleRateInv = 1 / sr;
		brightCoef = tan(jmin(briHold,flt.SampleRate*0.5f-10)* pi * flt.sampleRateInv);
		hpfcutoff = tan(hpffreq * sampleRateInv * pi);
	}
	void checkAdssrState()
	{
		shouldProcessed = env.isActive();
	}
	void ResetEnvelopes()
	{
		env.ResetEnvelopeState();
		fenv.ResetEnvelopeState();
	}
	void NoteOn(int mididx,float velocity,bool multiTrig,bool doPorta=true)
	{
		if(!shouldProcessed)
		{
			//When your processing is paused we need to clear delay lines and envelopes
			//Not doing this will cause clicks or glitches
			lenvd.fillZeroes();
			fenvd.fillZeroes();
			ResetEnvelopes();
		}
		shouldProcessed = true;
		if(velocity!=-0.5)
			// Scale velocity according to velscale [ 8..1..1/8 ]
			// range is same (0..1 -> 0..1), but scale changes
			velocityValue = powf(velocity, velscale);
		midiIndx = mididx;
		if(!Active || multiTrig) {
			if (envRst) {
				ResetEnvelopes();
				// Ramp down whatever is in the env delay lines
				// to zero to minimize clicking when envelopes
				// are reset.
				lenvd.decayLine();
				fenvd.decayLine();
			}
			env.triggerAttack();
			fenv.triggerAttack();
			lfo1.keyResetPhase();
			lfo2.keyResetPhase();
		}
		if (oscKeySync)
			osc.keyReset = true;
		Active = true;
		portaEnable = doPorta;
		porta = portaEnable ? portaSaved : 250;
	}
	void NoteOff()
	{
		if(!sustainHold) {
			env.triggerRelease();
			fenv.triggerRelease();
		}
		Active = false;
	}
	void sustOn()
	{
		sustainHold = true;
	}
	void sustOff()
	{
		sustainHold = false;
		if(!Active)
		{
			env.triggerRelease();
			fenv.triggerRelease();
		}

	}
};
