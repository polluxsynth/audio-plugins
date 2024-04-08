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

#include "Voice.h"
#include "Motherboard.h"
#include "Params.h"
#include "ParamSmoother.h"

#define ForEachVoice(expr) \
	for(int i = 0 ; i < synth.MAX_VOICES;i++) \
		{\
			synth.voices[i].expr;\
		}

class SynthEngine
{
private:
	Motherboard synth;
	ParamSmoother cutoffSmoother;
	ParamSmoother pitchWheelSmoother;
	ParamSmoother modWheelSmoother;
	ParamSmoother afterTouchSmoother;
	float sampleRate;
	float atscale;
	float velscale;
	// TODO Remove unused1,2:
	float unused1, unused2;
	//JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthEngine)

public:
	SynthEngine():
		cutoffSmoother(),
		//synth = new Motherboard();
		pitchWheelSmoother(),
		modWheelSmoother(),
		afterTouchSmoother()
	{
		atscale = 1;
		velscale = 1;
	}
	~SynthEngine()
	{
		//delete synth;
	}
	void setPlayHead(float bpm,float retrPos)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].lfo1.hostSyncRetrigger(bpm,retrPos);
			synth.voices[i].lfo2.hostSyncRetrigger(bpm,retrPos);
		}
	}
	void setSampleRate(float sr)
	{
		sampleRate = sr;
		cutoffSmoother.setSampleRate(sr);
		pitchWheelSmoother.setSampleRate(sr);
		modWheelSmoother.setSampleRate(sr);
		afterTouchSmoother.setSampleRate(sr);
		synth.setSampleRate(sr);
	}
	void processSample(float *left,float *right)
	{
		processCutoffSmoothed(cutoffSmoother.smoothStep());
		procPitchWheelSmoothed(pitchWheelSmoother.smoothStep());
		procModWheelSmoothed(modWheelSmoother.smoothStep());
		procAfterTouchSmoothed(afterTouchSmoother.smoothStep());

		synth.processSample(left,right);
	}
	void allNotesOff()
	{
		for(int i = 0 ;  i < 128;i++)
			procNoteOff(i);
	}
	void allSoundOff()
	{
		allNotesOff();
		ForEachVoice(ResetEnvelopes());
	}
	void sustainOn()
	{
		synth.sustainOn();
	}
	void sustainOff()
	{
		synth.sustainOff();
	}
	void setLfo1Sync(float val)
	{
		// Off - Tempo - Key - Oneshot
		int intval = roundToInt(val);
		for(int i = 0 ; i < Motherboard::MAX_VOICES;i++)
		{
			synth.voices[i].lfo1.setClockSync(intval == 1);
			synth.voices[i].lfo1.setKeySync(intval >= 2 && intval <= 3);
			synth.voices[i].lfo1.setOneShot(intval == 3);
		}
	}
	void setLfo2Sync(float val)
	{
		// Off - Tempo - Key - Oneshot
		int intval = roundToInt(val);
		for(int i = 0 ; i < Motherboard::MAX_VOICES;i++)
		{
			synth.voices[i].lfo2.setClockSync(intval == 1);
			synth.voices[i].lfo2.setKeySync(intval >= 2 && intval <= 3);
			synth.voices[i].lfo2.setOneShot(intval == 3);
		}
	}
	void setLfo1Polarity(float val)
	{
		// Normal - Invert - Unipolar - Unipolar+Invert
		int intval = roundToInt(val);
		ForEachVoice(lfo1.invert = (intval & 1));
		ForEachVoice(lfo1.unipolar = (intval & 2));
	}
	void setLfo2Polarity(float val)
	{
		// Normal - Invert - Unipolar - Unipolar+Invert
		int intval = roundToInt(val);
		ForEachVoice(lfo2.invert = (intval & 1));
		ForEachVoice(lfo2.unipolar = (intval & 2));
	}
	void setKeyAsgnRsz(float val)
	{
		synth.voiceAlloc.rsz = roundToInt(val);
	}
	void setKeyAsgnMem(float val)
	{
		synth.voiceAlloc.mem = roundToInt(val);
	}
	void setKeyAsgnRob(float val)
	{
		int intval = roundToInt(val);
		synth.voiceAlloc.rob_oldest = intval == 1;
		synth.voiceAlloc.rob_next_to_lowest = intval == 2;
	}
	void setKeyAsgnRes(float val)
	{
		synth.voiceAlloc.restore = roundToInt(val);
	}
	void setKeyAsgnStrg(float val)
	{
		int intval = roundToInt(val);
		synth.voiceAlloc.strgNoteOff = intval == 1 || intval == 2;
		synth.voiceAlloc.strgNoteOn = intval == 2;
	}
	void setKeyAsgnMode(float param)
	{
		// Poly - Mono - Mono+Auto [Portamento]
		int intval = roundToInt(param);
		synth.voiceAlloc.uni = intval >= 1;
		synth.voiceAlloc.alwaysPorta = intval < 2;
	}
	void procNoteOn(int noteNo,float velocity)
	{
		synth.voiceAlloc.setNoteOn(noteNo,velocity);
	}
	void procNoteOff(int noteNo)
	{
		synth.voiceAlloc.setNoteOff(noteNo);
	}
	void setEconomyMode(float val)
	{
		synth.economyMode = roundToInt(val);
	}

	// TODO: Remove
	void procUnused1(float val)
	{
		unused1=val;
		ForEachVoice(unused1=val);
		ForEachVoice(osc.unused1=val);
		ForEachVoice(flt.unused1=val);
		ForEachVoice(env.unused1=val);
		ForEachVoice(fenv.unused1=val);
	}
	// TODO: Remove
	void procUnused2(float val)
	{
		unused2=val;
		ForEachVoice(unused2=val);
		ForEachVoice(osc.unused2=val);
		ForEachVoice(flt.unused2=val);
		ForEachVoice(env.unused2=val);
		ForEachVoice(fenv.unused2=val);
	}
	void setAmpVelocityAmount(float val)
	{
		ForEachVoice(vamp = val * 0.1f);
	}
	void setFltVelocityAmount(float val)
	{
		ForEachVoice(vflt = val * 0.1f);
	}
	void setVelocityScale(float scale)
	{
		scale = 1 - 0.2f * scale; // 0..10 -> 1..0..-1
		scale = powf(8.0f, scale); // => 8 .. 1 .. 1/8
		ForEachVoice(velscale = scale);
	}
	void setAfterTouchScale(float scale)
	{
		scale = 1 - 0.2f * scale; // 0..10 -> 1..0..-1
		atscale = powf(8.0f, scale); // => 8 .. 1 .. 1/8
	}
	void procModWheel(float val)
	{
		modWheelSmoother.setSteep(val);
	}
	void procModWheelSmoothed(float val)
	{
		ForEachVoice(modw = val);
	}
	void procAfterTouch(float val)
	{
		val = powf(val, atscale);
		afterTouchSmoother.setSteep(val);
	}
	void procAfterTouchSmoothed(float val)
	{
		ForEachVoice(aftert = val);
	}
	void setLfo2Frequency(float val)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].lfo2.setRawFrequency(val);
			synth.voices[i].lfo2.setFrequency(logsc(val, 0, 100, 240));
		}
	}
	void procPitchWheel(float val)
	{
		pitchWheelSmoother.setSteep(val);
	}
	inline void procPitchWheelSmoothed(float val)
	{
		ForEachVoice(pitchWheel = val);
	}
	void setVoiceCount(float param)
	{
		synth.setVoiceCount(roundToInt(param));
	}
	void setPitchWheelAmount(float param)
	{
		int intparam = roundToInt(param);
		ForEachVoice(pitchWheelAmt = intparam);
	}
	void setPitchWheelDest(float param)
	{
		// OFF - OSC1 - OSC1+2
		int intparam = roundToInt(param);
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].pitchWheelOsc1 = intparam >= 1;
			synth.voices[i].pitchWheelOsc2 = intparam == 2;
		}
	}
	void setPanSpread(float param)
	{
		synth.SetPanSpreadAmt(param * 0.1f);
	}
	void setTune(float param)
	{
		ForEachVoice(osc.tune = param);
		ForEachVoice(filtertune = param);
	}
	void setOctave(float param)
	{
		// Add 2 before rounding to avoid problems around zero
		int octave = (roundToInt(param + 2.0f) - 2) * 12;
		ForEachVoice(osc.oct = octave);
		ForEachVoice(filteroct = octave);
	}
	void setFilterKeyFollow(float param)
	{
		ForEachVoice(fltKF = param);
	}
	void setPortamento(float param)
	{
		float porta = timesc(10 - param, 0.14f, 250);

		ForEachVoice(setPorta(porta));
	}
	void setVolume(float param)
	{
		synth.Volume = linsc(param, 0, 0.30f);
	}
	void setLfo1Frequency(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].lfo1.setRawFrequency(param);
			synth.voices[i].lfo1.setFrequency(logsc(param, 0, 100, 240));
		}
	}
	void setLfo1Wave(float param)
	{
		int intparam = roundToInt(param);
		ForEachVoice(lfo1.setWaveForm(intparam));
	}
	void setLfo2Wave(float param)
	{
		int intparam = roundToInt(param);
		ForEachVoice(lfo2.setWaveForm(intparam));
	}
	void setLfo1Amt(float param)
	{
		ForEachVoice(lfo1amt = param * 0.1f);
	}
	void setLfo1Dest(float param)
	{
		int intparam = roundToInt(param);
		// off, osc1, osc1+2, osc2, pw1, pw1+2, pw2, filt, res, bmod
		// 0    1     2       3     4    5      6    7     8    9
		bool lfo1o1 = intparam == 1 || intparam == 2;
		bool lfo1o2 = intparam == 2 || intparam == 3;
		bool lfo1pw1 = intparam == 4 || intparam == 5;
		bool lfo1pw2 = intparam == 5 || intparam == 6;
		bool lfo1filt = intparam == 7;
		bool lfo1res = intparam == 8;
		bool lfo1bmod = intparam == 9;
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].lfo1o1 = lfo1o1;
			synth.voices[i].lfo1o2 = lfo1o2;
			synth.voices[i].lfo1pw1 = lfo1pw1;
			synth.voices[i].lfo1pw2 = lfo1pw2;
			synth.voices[i].lfo1f = lfo1filt;
			synth.voices[i].lfo1res = lfo1res;
			synth.voices[i].lfo1bmod = lfo1bmod;
		}
	}
	void setLfo2Dest(float param)
	{
		int intparam = roundToInt(param);
		// off, osc1, osc1+2, osc2, pw1, pw1+2, pw2, filt, res, bmod
		// 0    1     2       3     4    5      6    7     8    9
		bool lfo2o1 = intparam == 1 || intparam == 2;
		bool lfo2o2 = intparam == 2 || intparam == 3;
		bool lfo2pw1 = intparam == 4 || intparam == 5;
		bool lfo2pw2 = intparam == 5 || intparam == 6;
		bool lfo2filt = intparam == 7;
		bool lfo2res = intparam == 8;
		bool lfo2bmod = intparam == 9;
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].lfo2o1 = lfo2o1;
			synth.voices[i].lfo2o2 = lfo2o2;
			synth.voices[i].lfo2pw1 = lfo2pw1;
			synth.voices[i].lfo2pw2 = lfo2pw2;
			synth.voices[i].lfo2f = lfo2filt;
			synth.voices[i].lfo2res = lfo2res;
			synth.voices[i].lfo2bmod = lfo2bmod;
		}
	}
	void setLfo1Controller(float val)
	{
		int intval = roundToInt(val);
		// off - modwheel - aftertouch - lfo2
		ForEachVoice(lfo1modw = (intval == 1));
		ForEachVoice(lfo1after = (intval == 2));
		ForEachVoice(lfo1vel = (intval == 3));
	}
	void setLfo1ControllerAmt(float val)
	{
		ForEachVoice(lfo1modamt = val * 0.1f);
	}
	void setLfo2Controller(float val)
	{
		int intval = roundToInt(val);
		// off - modwheel - aftertouch - lfo1
		ForEachVoice(lfo2modw = (intval == 1));
		ForEachVoice(lfo2after = (intval == 2));
		ForEachVoice(lfo2vel = (intval == 3));
	}
	void setLfo2ControllerAmt(float val)
	{
		ForEachVoice(lfo2modamt = val * 0.1f);
	}
	void setLfo2Amt(float param)
	{
		ForEachVoice(lfo2amt = param * 0.1f);
	}
	void setOscSpread(float param)
	{
		ForEachVoice(osc.totalSpread = logsc(param, 0.001f, 0.90f));
	}
	void setOsc1PulseWidth(float param)
	{
		ForEachVoice (osc.osc1pw = param * 0.1f);
	}
	void setOsc2PulseWidth(float param)
	{
		ForEachVoice (osc.osc2pw = param * 0.1f);
	}
	void setInvertFenv(float param)
	{
		ForEachVoice(invertFenv = roundToInt(param));
	}
	void setFenvLinear(float param)
	{
		ForEachVoice(fenv.setLinear(roundToInt(param)));
	}
	void setEnvMode(float param)
	{
		// Exp env / Lin VCA - Lin env / Lin VCA - Lin env / Exp VCA
		int intparam = roundToInt(param);
		ForEachVoice(env.setLinear(intparam >= 1));
		ForEachVoice(expvca = (intparam >= 2));
	}
	void setOsc2Xmod(float param)
	{
		ForEachVoice(osc.xmod = param * 2.4f);
	}
	void setOsc2SyncLevel(float param)
	{
		ForEachVoice(osc.syncLevel = 1.0f - param * 0.1f);
	}
	void setOsc1Pitch(float param)
	{
		ForEachVoice(osc.osc1p = roundToInt(param));
	}
	void setOsc2Pitch(float param)
	{
		ForEachVoice(osc.osc2p = roundToInt(param));
	}
	void setOsc1Mix(float param)
	{
		ForEachVoice(osc.o1mx = param * 0.1f);
	}
	void setOsc2Mix(float param)
	{
		ForEachVoice(osc.o2mx = param * 0.1f);
	}
	void setHPFfreq(float param)
	{
		ForEachVoice(setHPFfreq(logsc(param, 4, 2500)));
	}
	void setVCADrive(float param)
	{
		ForEachVoice(sqdist.setAmount(param * 0.0435f));
	}
	void setOsc2FltMod(float param)
	{
		ForEachVoice(osc2FltMod = param * 10);
	}
	void setOsc1Det(float param)
	{
		// logsc is geared for 0..10. so we multiply param by 10
		// as the displayed parameter value is 0..1.
		ForEachVoice(osc.osc1Det = linsc(param * 10, 0, 1.0f));
	}
	void setOsc2Det(float param)
	{
		ForEachVoice(osc.osc2Det = linsc(param * 10, 0, 1.0f));
	}

	void setOsc1Wave(float param)
	{
		int intparam = roundToInt(param);
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.osc1Saw = intparam == 1;
			synth.voices[i].osc.osc1Pul = intparam == 2;
			synth.voices[i].osc.osc1Tri = intparam == 3;
		}
	}

	void setOsc2Wave(float param)
	{
		int intparam = roundToInt(param);
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.osc2Saw = intparam == 1;
			synth.voices[i].osc.osc2Pul = intparam == 2;
			synth.voices[i].osc.osc2Tri = intparam == 3;
			synth.voices[i].osc.osc2modout =
			synth.voices[i].oscmodEnable = intparam != 0;
		}
	}
	void setOsc3Wave(float param)
	{
		int intparam = roundToInt(param);
		// off - -1 square - -2 square - -2 pulse - noise
		ForEachVoice(osc.osc3Waveform = intparam);
	}
	void setOsc3Mix(float param)
	{
		ForEachVoice(osc.o3mx = param * 0.1f);
	}
	void setCutoff(float param)
	{
		cutoffSmoother.setSteep(linsc(param, 0, 120));
	}
	inline void processCutoffSmoothed(float param)
	{
		ForEachVoice(cutoff=param);
	}
	void setResonance(float param)
	{
		ForEachVoice(res = linsc(param,0, 0.991f));
	}
	void setResponse(float param)
	{
		// Pole count 1 .. 4 (continuous)
		ForEachVoice(flt.setResponse(4 - param));
	}
	void setOversampling(float param)
	{
		synth.SetOversample(roundToInt(param));
	}
	void setFilterEnvelopeAmt(float param)
	{
		// Linearly scaled to (+/-) 0..70 semitones
		ForEachVoice(fenvamt = linsc(param, 0, 70));
	}
	void setLoudnessEnvelopeAttack(float param)
	{
		ForEachVoice(env.setAttack(timesc(param, 1, 12500)));
	}
	void setLoudnessEnvelopeDecay(float param)
	{
		ForEachVoice(env.setDecay(timesc(param, 1, 20500)));
	}
	void setLoudnessEnvelopeSustainTime(float param)
	{
		ForEachVoice(env.setSustainTime(timesc(param, 1, 41000)));
		// When time is set to 1.0, sustain time is infinite
		ForEachVoice(env.setAdsr(param > 9.91f));
	}
	void setLoudnessEnvelopeRelease(float param)
	{
		ForEachVoice(env.setRelease(timesc(param, 1, 41000)));
	}
	void setLoudnessEnvelopeSustain(float param)
	{
		ForEachVoice(env.setSustain(param * 0.1f));
	}
	void setFilterEnvelopeAttack(float param)
	{
		ForEachVoice(fenv.setAttack(timesc(param, 1, 12500)));
	}
	void setFilterEnvelopeDecay(float param)
	{
		ForEachVoice(fenv.setDecay(timesc(param, 1, 20500)));
	}
	void setFilterEnvelopeSustainTime(float param)
	{
		ForEachVoice(fenv.setSustainTime(timesc(param, 1, 41000)));
		// When time is set to 1.0, sustain time is infinite
		ForEachVoice(fenv.setAdsr(param > 9.91f));
	}
	void setFilterEnvelopeRelease(float param)
	{
		ForEachVoice(fenv.setRelease(timesc(param, 1, 41000)));
	}
	void setFilterEnvelopeSustain(float param)
	{
		ForEachVoice(fenv.setSustain(param * 0.1f));
	}
	void setEnvelopeSpread(float param)
	{
		ForEachVoice(setEnvSpreadAmt(linsc(param, 0.0f, 1.0f)));
	}
	void setLfoSpread(float param)
	{
		ForEachVoice(setLfoSpreadAmt(linsc(param, 0, 1)));
	}
	void setFilterSpread(float param)
	{
		ForEachVoice(FltSpreadAmt = linsc(param, 0, 18));
	}
	void setPortamentoSpread(float param)
	{
		ForEachVoice(PortaSpreadAmt = linsc(param, 0.0f, 0.75f));
	}
	void setLoudnessSpread(float param)
	{
		ForEachVoice(levelSpreadAmt = linsc(param, 0.0f, 0.67f));
	}
	void setOscKeySync(float param)
	{
		ForEachVoice(oscKeySync = roundToInt(param));
	}
	void setEnvRst(float param)
	{
		ForEachVoice(envRst = roundToInt(param));
	}
};
