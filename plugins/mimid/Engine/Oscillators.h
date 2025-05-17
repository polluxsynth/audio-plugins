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
#include "Voice.h"
#include "SynthEngine.h"
#include "AudioUtils.h"
#include "BlepData.h"
#include "DelayLine.h"
#include "SawOsc.h"
#include "PulseOsc.h"
#include "TriangleOsc.h"
#include "TrapezoidOsc.h"
#include "VariSawOsc.h"
#include "SubOsc.h"

class Oscillators
{
private:
	float SampleRate;
	float sampleRateInv;
	// MinTraSymmetry: unit: sample interval
	static constexpr float MinTraSymmetry = 0.15f; /* Empirically determined */
	// SawMinSlopeus: unit: s
	static constexpr float SawMinSlopeus = 58.0e-6f; /* Empirically determined */
	// Derating factor when saw slope is < SawMinSlopeus
	static constexpr float SawMinDerate = 0.1f;
	float SawMaxGrad;

	float x1, x2;

	float osc1Factor;
	float osc2Factor;

	//blep const
	const int n;
	const int hsam;
	//delay line implements fixed sample delay
	DelayLine<Samples> osc2d;
	DelayLineInt<Samples> syncd;
	DelayLine<Samples> syncFracd;
	DelayLine<Samples> cvd;
	SRandom wn;
	Antialias o1aa, o2aa, subaa;
	SawOsc o1s, o2s;
	PulseOsc o1p, o2p;
	TriangleOsc o1t, o2t;
	TrapezoidOsc o1z, o2z;
	VariSawOsc o1v, o2v;
	SubOsc o2sub;
public:

	float tune; //+-1
	int oct;

	float dirt;

	float notePlaying;


	float totalSpread;

	float osc1Det, osc2Det;
	float osc1pw, osc2pw;
	float pw1, pw2;

	float o1mx, o2mx, o2submx;
	float nmx;
	float pto1, pto2;

	//osc waveshapes
	int osc1Wave, osc2Wave;
	int osc2SubWaveform;

	// Osc waveshaping parameters
	// These are calculated at the mod rate by Voice
	// Pulse wave
	float pw1calc, pw2calc;
	// Trapezoid wave
	float symmetry1, symmetry2;
	// VariSaw wave
	float sgradient1, sgradient2;

	float osc1p,osc2p;
	float syncLevel;
	float xmod;
	bool osc2modout;

	bool keyReset;

	float unused1, unused2; //TODO remove

	Oscillators() :
		n(Samples * 2),
		hsam(Samples),
		o1aa(), o2aa(), subaa(),
		o1s(o1aa), o2s(o2aa),
		o1p(o1aa), o2p(o2aa),
		o1t(o1aa), o2t(o2aa),
		o1z(o1aa), o2z(o2aa),
		o1v(o1aa), o2v(o2aa),
		o2sub(subaa)
	{
		dirt = 0.1;
		totalSpread = 0;
		wn = SRandom(SRandom::globalRandom().nextInt32());
		osc1Factor = wn.nextFloat() - 0.5f;
		osc2Factor = wn.nextFloat() - 0.5f;
		nmx = 0;
		oct = 0;
		tune = 0;
		pto1 = pto2 = 0;
		pw1 = pw2 = 0;
		pw1calc = pw2calc = 0;
		symmetry1 = symmetry2 = 0;
		sgradient1 = sgradient2 = 1;
		xmod = 0;
		syncLevel = 1.0f;
		osc1p = osc2p = 24.0f;
		osc1Wave = osc2Wave = 0;
		osc1Det = osc2Det = 0;
		notePlaying = 30;
		osc1pw = osc2pw = 0;
		o1mx = o2mx = 0;
		x1 = wn.nextFloat();
		x2 = wn.nextFloat(); // osc2 and 3 start in phase
		SawMaxGrad = 1.0f;
		osc2SubWaveform = 0; // off
		keyReset = false;
	}
	~Oscillators()
	{
	}
	void setDecimation()
	{
		o1aa.setDecimation();
		o2aa.setDecimation();
		subaa.setDecimation();
	}
	void removeDecimation()
	{
		o1aa.removeDecimation();
		o2aa.removeDecimation();
		subaa.removeDecimation();
	}
	void setSampleRate(float sr)
	{
		SampleRate = sr;
		sampleRateInv = 1.0f / SampleRate;
		SawMaxGrad = 1.0f / (SawMinSlopeus * sr);
	}
	inline void ProcessSample(float &audioOutput, float &modOutput)
	{
		// osc 2 = master oscillator
		float noiseGen = wn.nextFloat() - 0.5f;
		float pitch2 = getPitch(dirt * noiseGen + notePlaying + osc2Det + osc2p + pto2 + tune + oct + totalSpread * osc2Factor);
		// hard sync is subject to sync level parameter
		// osc key sync results in unconditional hard sync
		int hsr = 0; // 1 => hard sync, -1 => unconditional hard sync
		float hsfrac = 0.0f;
		float fs = minf(pitch2 * sampleRateInv, 0.45f);
		x2 += fs;
		float osc2mix = 0.0f;

#define PhaseResetMaster(x2, fs, hsr, hsfrac, keyReset) \
		if (keyReset) { \
			x2 = 0.0f; \
			hsfrac = 1.0f; \
			hsr = -1; /* unconditional hard sync */ \
		} else if (x2 >= 1.0f) { \
			x2 -= 1.0f; \
			hsfrac = x2 / fs; \
			hsr = 1; /* hard sync governed by sync level */ \
		}

		switch (osc2Wave) {
		case 2: // Pulse
			o2p.processMaster(x2, fs, pw2calc, keyReset);
			PhaseResetMaster(x2, fs, hsr, hsfrac, keyReset);
			osc2mix = o2p.getValue(x2, pw2calc);
			break;
		case 3: // Triangle
			o2t.processMaster(x2, fs, keyReset);
			PhaseResetMaster(x2, fs, hsr, hsfrac, keyReset);
			osc2mix = o2t.getValue(x2);
			break;
		case 4:	{ // Trapezoid
			float symmetry = limitf(symmetry2, MinTraSymmetry * fs, 1 - MinTraSymmetry * fs);
			// With limitation above, don't need 0 check here
			float riseGradient = 1.0f / symmetry;
			float fallGradient = 1.0f / (symmetry - 1.0f);

			o2z.processMaster(x2, fs, symmetry, riseGradient, fallGradient, keyReset);
			PhaseResetMaster(x2, fs, hsr, hsfrac, keyReset);
			osc2mix = o2z.getValue(x2, symmetry, riseGradient, fallGradient);
			}
			break;
		case 5:	if (sgradient2 != 1.0f) { // VariSaw
			float grad_limit = SawMaxGrad / fs;
			// Above a certain gradient, the waveform amplitude
			// starts decreasing. We could put a hard limit here,
			// but that would cause the resulting parameter to
			// hit a brick wall when increased which gives a
			// confusing user experience. Therefore, derate
			// the increase instead, only bringing in 10% of the
			// increase above the gradient limit, so that there
			// still some albeit much less severe change in the
			// waveform all the way to the max parameter value.
			float grad_derate = sgradient2 < grad_limit ? 1.0f : SawMinDerate;
			float sgrad = (sgradient2 - grad_limit) * grad_derate + grad_limit;
			// breakpoint is 1 / gradient, but only when
			// gradient is > 1. Otherwise it is 1 (maxed).
			// The normal case here is 1 / gradient, so optimize
			// for that, with the lack of branches ensuring
			// that it is no worse for the more unusual case.
			float dividend = sgrad > 1.0f ? 1.0f : sgrad;
			float sbreakpoint = dividend / sgrad;
			o2v.processMaster(x2, fs, sbreakpoint, sgrad, keyReset);
			PhaseResetMaster(x2, fs, hsr, hsfrac, keyReset);
			osc2mix = o2v.getValue(x2, sbreakpoint, sgrad);
			break;
			}
			[[fallthrough]];
		case 1: // Saw
			// Reset state of variable slope sawtooth oscillator
			// so we can quickly move over to it if gradient
			// changes from 1.0 .
			o2v.resetState();
			o2s.processMaster(x2, fs, keyReset);
			PhaseResetMaster(x2, fs, hsr, hsfrac, keyReset);
			osc2mix = o2s.getValue(x2);
			break;
		case 0: // Off
		default:
			PhaseResetMaster(x2, fs, hsr, hsfrac, keyReset);
			break;
		}

		// Delaying our hard sync gate signal and frac
		hsr = syncd.feedReturn(hsr);
		hsfrac = syncFracd.feedReturn(hsfrac);

		// osc2sub: osc2 sub oscillator
		float osc2submix = 0.0f;

		noiseGen = wn.nextFloat()-0.5; // for noise + osc1 dirt + mix dither

		// Send hard sync reset as trigger for sub osc counter
		// Because they're delayed above, we don't need to
		// delay the output of sub osc further down.
		o2sub.processMaster(hsr, hsfrac, osc2SubWaveform);

		if (osc2SubWaveform) {
			if (osc2SubWaveform == 4) { // noise
				// MiMi-a uses a digital noise generator,
				// so we do too. It has the minimum crest
				// factor and thus gives the highest RMS level
				// for a given peak level.
				// TODO: Use separate noise gen here?
				osc2submix = (noiseGen > 0) - 0.5;
				// osc2submix = noiseGen * 1.3; // analog
			} else // 1..3 are sub osc waveforms/octaves
				osc2submix = o2sub.getValue(osc2SubWaveform);
		}

		// osc1 = slave oscillator

		// Pitch control needs additional delay buffer to compensate
		// This will give us less aliasing on xmod
		// Hard sync gate signal delayed too
		// Offset on osc2mix * xmod is to get zero pitch shift at
		// max xmod
		float pitch1 = getPitch(cvd.feedReturn(dirt *noiseGen + notePlaying + osc1Det + osc1p + pto1 + (osc2modout?osc2mix-0.0569:0)*xmod + tune + oct +totalSpread*osc1Factor));

		fs = minf(pitch1 * sampleRateInv, 0.45f);

		float osc1mix = 0.0f;

		x1 += fs;

		// Sync level: we check if x1 is above the sync level when
		// the reset from the master oscillator occurs.
		// If the sync level is high enough, disable completely,
		// to avoid artefacts when osc2 is running at an insanely
		// high frequency (empirically).
		// For osc key sync, hard sync is unconditional
		if (hsr > 0) // hard sync
			hsr &= (syncLevel <= 0.99f) && (x1 - hsfrac * fs >= syncLevel);

#define PhaseResetSlave(x1, fs, hsr, hsfrac) \
		if (hsr) { \
			/* On hard sync reset slave phase to master frac */ \
			float fracMaster = fs * hsfrac; \
			x1 = fracMaster; \
		} else if (x1 >= 1.0f) \
			x1 -= 1.0f; \

		switch (osc1Wave) {
		case 2: // Pulse
			o1p.processSlave(x1, fs, hsr, hsfrac, pw1calc);
			PhaseResetSlave(x1, fs, hsr, hsfrac);
			osc1mix = o1p.getValue(x1, pw1calc);
			break;
		case 3: // Triangle
			o1t.processSlave(x1, fs, hsr, hsfrac);
			PhaseResetSlave(x1, fs, hsr, hsfrac);
			osc1mix = o1t.getValue(x1);
			break;
		case 4:	{ // Trapezoid
			float symmetry = limitf(symmetry1, MinTraSymmetry * fs, 1 - MinTraSymmetry * fs);
			// With limitation above, don't need 0 check here
			float riseGradient = 1.0f / symmetry;
			float fallGradient = 1.0f / (symmetry - 1.0f);
			o1z.processSlave(x1, fs, hsr, hsfrac, symmetry, riseGradient, fallGradient);
			PhaseResetSlave(x1, fs, hsr, hsfrac);
			osc1mix = o1z.getValue(x1, symmetry, riseGradient, fallGradient);
			}
			break;
		case 5:	if (sgradient1 != 1.0f) { // VariSaw
			float grad_limit = SawMaxGrad / fs;
			float grad_derate = sgradient1 < grad_limit ? 1.0f : SawMinDerate;
			float sgrad = (sgradient1 - grad_limit) * grad_derate + grad_limit;
			float dividend = sgrad > 1.0f ? 1.0f : sgrad;
			float sbreakpoint = dividend / sgrad;
			o1v.processSlave(x1, fs, hsr, hsfrac, sbreakpoint, sgrad);
			PhaseResetSlave(x1, fs, hsr, hsfrac);
			osc1mix = o1v.getValue(x1, sbreakpoint, sgrad);
			break;
			}
			[[fallthrough]];
		case 1: // Saw
			// Reset state of variable slope sawtooth oscillator
			// so we can quickly move over to it if gradient
			// changes from 1.0 .
			o1v.resetState();
			o1s.processSlave(x1, fs, hsr, hsfrac);
			PhaseResetSlave(x1, fs, hsr, hsfrac);
			osc1mix = o1s.getValue(x1);
			break;
		case 0: // Off
		default:
			PhaseResetSlave(x1, fs, hsr, hsfrac);
			break;
		}

		// Delay osc2 to get in phase with osc1 which is
		// in itself delayed due to delay after pitch calc.
		// TOOD: Review this: Should the xmod really be delayed
		// in the osc1 pitch calc, it would seem to be one delay
		// too many in the xmod path.
		osc2mix = osc2d.feedReturn(osc2mix);

		//mixing
		// TODO: have separate noise generator for the dither noise?
		float res = o1mx*osc1mix + o2mx*osc2mix + o2submx*osc2submix + noiseGen*0.0006;
		audioOutput = res * 3.0f;
		modOutput = osc2mix;

		keyReset = false;
	}
};
