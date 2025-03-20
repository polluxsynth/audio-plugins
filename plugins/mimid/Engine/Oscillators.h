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
	static constexpr float SawMinSlopeus = 28.0e-6f; /* Empirically determined */
	float SawMinSlope;

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
	bool osc1Saw, osc1Pul, osc1Tri, osc1Tra, osc1Var;
	bool osc2Saw, osc2Pul, osc2Tri, osc2Tra, osc2Var;
	int osc2SubWaveform;

	// Osc waveshaping parameters
	// These are calculated at the mod rate by Voice
	// Pulse wave
	float pw1calc, pw2calc;
	// Trapezoid wave
	float symmetry1, symmetry2;
	// VariSaw wave
	float ssymmetry1, ssymmetry2;

	float osc1p,osc2p;
	float syncLevel;
	float xmod;
	bool osc2modout;

	bool keyReset;

	float unused1, unused2; //TODO remove

	Oscillators() :
		n(Samples * 2),
		hsam(Samples),
		o1s(), o2s(),
		o1p(), o2p(),
		o1t(), o2t(),
		o1z(), o2z(),
		o1v(), o2v()
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
		ssymmetry1 = ssymmetry2 = 0;
		xmod = 0;
		syncLevel = 1.0f;
		osc1p = osc2p = 24.0f;
		osc1Saw = osc2Saw = osc1Pul = osc2Pul = false;
		osc1Det = osc2Det = 0;
		notePlaying = 30;
		osc1pw = osc2pw = 0;
		o1mx = o2mx = 0;
		x1 = wn.nextFloat();
		x2 = wn.nextFloat(); // osc2 and 3 start in phase
		SawMinSlope = 1.0f;
		osc2SubWaveform = 0; // off
		keyReset = false;
	}
	~Oscillators()
	{
	}
	void setDecimation()
	{
		o1p.setDecimation();
		o1t.setDecimation();
		o1s.setDecimation();
		o1z.setDecimation();
		o1v.setDecimation();
		o2p.setDecimation();
		o2t.setDecimation();
		o2s.setDecimation();
		o2z.setDecimation();
		o2v.setDecimation();
		o2sub.setDecimation();
	}
	void removeDecimation()
	{
		o1p.removeDecimation();
		o1t.removeDecimation();
		o1s.removeDecimation();
		o1z.removeDecimation();
		o1v.removeDecimation();
		o2p.removeDecimation();
		o2t.removeDecimation();
		o2s.removeDecimation();
		o2z.removeDecimation();
		o2v.removeDecimation();
		o2sub.removeDecimation();
	}
	void setSampleRate(float sr)
	{
		SampleRate = sr;
		sampleRateInv = 1.0f / SampleRate;
		SawMinSlope = SawMinSlopeus * sr; // In units of sample int'vl
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

		// Local variables for waveshaping. Set for antialising,
		// reused for same oscillator's waveform calculation,
		// then reused for other oscillator.
		float symmetry = 0, riseGradient = 0, fallGradient = 0;
		float ssymmetry = 0, breakpoint = 0, sgradient = 0;

		if (osc2Pul) {
			o2p.processMaster(x2, fs, pw2calc, keyReset);
		} else if (osc2Saw) {
			o2s.processMaster(x2, fs, keyReset);
		} else if (osc2Tri) {
			o2t.processMaster(x2, fs, keyReset);
		} else if (osc2Tra) {
			symmetry = limitf(symmetry2, MinTraSymmetry * fs, 1 - MinTraSymmetry * fs);
			// With limitation above, don't need 0 check here
			riseGradient = 1.0f / symmetry;
			fallGradient = 1.0f / (symmetry - 1.0f);

			o2z.processMaster(x2, fs, symmetry, riseGradient, fallGradient, keyReset);
		} else if (osc2Var) {
			float symm_limit = SawMinSlope * fs;
			// We can't allow limit for ssymmetry to go above 1.
			if (symm_limit > 1) symm_limit = 1;
			ssymmetry = limitf(ssymmetry2, -1.0f + symm_limit, 1.0f - symm_limit);
			// breakpoint is the break point between slope and straight
			// part of waveform, regardless if symmetry is > 0 or < 0
			breakpoint = ssymmetry + (ssymmetry < 0);
			// Range limited, so no need to check here
			sgradient = 1.0f / (1.0f - fabsf(ssymmetry));

			o2v.processMaster(x2, fs, ssymmetry, breakpoint, sgradient, keyReset);
		}

		if (keyReset) {
			x2 = 0.0f;
			hsfrac = 1.0f;
			hsr = -1; // unconditional hard sync
		}
		else if (x2 >= 1.0f)
		{
			x2 -= 1.0f;
			hsfrac = x2 / fs;
			hsr = 1; // hard sync governed by sync level
		}

		// Delaying our hard sync gate signal and frac
		hsr = syncd.feedReturn(hsr);
		hsfrac = syncFracd.feedReturn(hsfrac);

		if (osc2Pul)
			osc2mix = o2p.getValue(x2, pw2calc);
		else if (osc2Saw)
			osc2mix = o2s.getValue(x2);
		else if (osc2Tri)
			osc2mix = o2t.getValue(x2);
		else if (osc2Tra)
			osc2mix = o2z.getValue(x2, symmetry, riseGradient, fallGradient);
		else if (osc2Var)
			osc2mix = o2v.getValue(x2, ssymmetry, breakpoint, sgradient);

		// osc2sub: osc2 sub oscillator
		noiseGen = wn.nextFloat()-0.5; // for noise + osc1 dirt + mix dither

		float osc2submix = 0.0f;

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

		if (osc1Pul) {
			o1p.processSlave(x1, fs, hsr, hsfrac, pw1calc);
		} else if (osc1Saw) {
			o1s.processSlave(x1, fs, hsr, hsfrac);
		} else if (osc1Tri) {
			o1t.processSlave(x1, fs, hsr, hsfrac);
		} else if (osc1Tra) {
			symmetry = limitf(symmetry1, MinTraSymmetry * fs, 1 - MinTraSymmetry * fs);
			// With limitation above, don't need 0 check here
			riseGradient = 1.0f / symmetry;
			fallGradient = 1.0f / (symmetry - 1.0f);
			o1z.processSlave(x1, fs, hsr, hsfrac, symmetry, riseGradient, fallGradient);
		} else if (osc1Var) {
			float symm_limit = SawMinSlope * fs;
			// We can't allow limit for ssymmetry to go above 1.
			if (symm_limit > 1) symm_limit = 1;
			ssymmetry = limitf(ssymmetry1, -1.0f + symm_limit, 1.0f - symm_limit);
			// breakpoint is the break point between slope and straight
			// part of waveform, regardless if symmetry is > 0 or < 0
			breakpoint = ssymmetry + (ssymmetry < 0);
			// Range limited, so no need to check here
			sgradient = 1.0f / (1.0f - fabsf(ssymmetry));

			o1v.processSlave(x1, fs, hsr, hsfrac, ssymmetry, breakpoint, sgradient);
		}

		if (x1 >= 1.0f)
			x1 -= 1.0f;

		// On hard sync reset slave phase is affected that way
		if (hsr)
		{
			float fracMaster = fs * hsfrac;
			x1 = fracMaster;
		}

		// Delay osc2 to get in phase with osc1 which is
		// in itself delayed due to delay after pitch calc.
		// TOOD: Review this: Should the xmod really be delayed
		// in the osc1 pitch calc, it would seem to be one delay
		// too many in the xmod path.
		osc2mix = osc2d.feedReturn(osc2mix);

		if (osc1Pul)
			osc1mix = o1p.getValue(x1, pw1calc);
		else if (osc1Saw)
			osc1mix = o1s.getValue(x1);
		else if (osc1Tri)
			osc1mix = o1t.getValue(x1);
		else if (osc1Tra)
			osc1mix = o1z.getValue(x1, symmetry, riseGradient, fallGradient);
		else if (osc1Var)
			osc1mix = o1v.getValue(x1, ssymmetry, breakpoint, sgradient);

		//mixing
		// TODO: have separate noise generator for the dither noise?
		float res = o1mx*osc1mix + o2mx*osc2mix + o2submx*osc2submix + noiseGen*0.0006;
		audioOutput = res * 3.0f;
		modOutput = osc2mix;

		keyReset = false;
	}
};
