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
			{
				procNoteOff(i);
			}
	}
	void allSoundOff()
	{
		allNotesOff();
		for(int i = 0 ; i < Motherboard::MAX_VOICES;i++)
			{
				synth.voices[i].ResetEnvelopes();
			}
	}
	void sustainOn()
	{
		synth.sustainOn();
	}
	void sustainOff()
	{
		synth.sustainOff();
	}
	void procLfo1Sync(float val)
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
	void procLfo2Sync(float val)
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
	void procLfo1Polarity(float val)
	{
		// Normal - Invert - Unipolar - Unipolar+Invert
		int intval = roundToInt(val);
		ForEachVoice(lfo1.invert = (intval & 1));
		ForEachVoice(lfo1.unipolar = (intval & 2));
	}
	void procLfo2Polarity(float val)
	{
		// Normal - Invert - Unipolar - Unipolar+Invert
		int intval = roundToInt(val);
		ForEachVoice(lfo2.invert = (intval & 1));
		ForEachVoice(lfo2.unipolar = (intval & 2));
	}
	void procKeyAsgnRsz(float val)
	{
		synth.voiceAlloc.rsz = roundToInt(val);
	}
	void procKeyAsgnMem(float val)
	{
		synth.voiceAlloc.mem = roundToInt(val);
	}
	void procKeyAsgnRob(float val)
	{
		int intval = roundToInt(val);
		synth.voiceAlloc.rob_oldest = intval == 1;
		synth.voiceAlloc.rob_next_to_lowest = intval == 2;
	}
	void procKeyAsgnRes(float val)
	{
		synth.voiceAlloc.restore = roundToInt(val);
	}
	void procKeyAsgnStrg(float val)
	{
		int intval = roundToInt(val);
		synth.voiceAlloc.strgNoteOff = intval == 1 || intval == 2;
		synth.voiceAlloc.strgNoteOn = intval == 2;
	}
	void processUnison(float param)
	{
		synth.voiceAlloc.uni = param>0.5f;
	}
	void procNoteOn(int noteNo,float velocity)
	{
		synth.voiceAlloc.setNoteOn(noteNo,velocity);
	}
	void procNoteOff(int noteNo)
	{
		synth.voiceAlloc.setNoteOff(noteNo);
	}
	void procEconomyMode(float val)
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
	void procAmpVelocityAmount(float val)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].vamp= val;
		}
	}
	void procFltVelocityAmount(float val)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].vflt= val;
		}
	}
	void procVelocityScale(float scale)
	{
		scale = 1 - 2 * scale; // 0..1 -> 1..0..-1
		scale = powf(8.0, scale); // => 8 .. 1 .. 1/8
		ForEachVoice(velscale = scale);
	}
	void procAfterTouchScale(float scale)
	{
		scale = 1 - 2 * scale; // 0..1 -> 1..0..-1
		atscale = powf(8.0, scale); // => 8 .. 1 .. 1/8
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
	void processLfo2Frequency(float val)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].lfo2.setRawFrequency(val);
			synth.voices[i].lfo2.setFrequency(logsc(val,0,100,240));
		}
	}
	void procPitchWheel(float val)
	{
		pitchWheelSmoother.setSteep(val);
		//for(int i = 0 ; i < synth->MAX_VOICES;i++)
		//{
		//	synth->voices[i]->pitchWheel = val;
		//}
	}
	inline void procPitchWheelSmoothed(float val)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].pitchWheel = val;
		}
	}
	void setVoiceCount(float param)
	{
		synth.setVoiceCount(roundToInt(param) + 1);
	}
	void procPitchWheelAmount(float param)
	{
		int intparam = roundToInt(param);
		ForEachVoice(pitchWheelAmt = intparam);
	}
	void procPitchWheelDest(float param)
	{
		// OFF - OSC1 - OSC1+2
		int intparam = roundToInt(param);
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].pitchWheelOsc1 = intparam >= 1;
			synth.voices[i].pitchWheelOsc2 = intparam == 2;
		}
	}
	void processPanSpread(float param)
	{
		synth.SetPanSpreadAmt(param);
	}
	void processTune(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.tune = param*2-1;
		}
	}
	void processOctave(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.oct = (roundToInt(param) - 2) * 12;
		}
	}
	void processFilterKeyFollow(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].fltKF = param;
		}
	}
	void processPortamento(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].porta =logsc(1-param,0.14,250,150);
		}
	}
	void processVolume(float param)
	{
		synth.Volume = linsc(param,0,0.30);
	}
	void processLfo1Frequency(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].lfo1.setRawFrequency(param);
			synth.voices[i].lfo1.setFrequency(logsc(param,0,100,240));
		}
	}
	void processLfo1Wave(float param)
	{
		int intparam = roundToInt(param);
		ForEachVoice(lfo1.setWaveForm(intparam));
	}
	void processLfo2Wave(float param)
	{
		int intparam = roundToInt(param);
		ForEachVoice(lfo2.setWaveForm(intparam));
	}
	void processLfo1Amt(float param)
	{
		ForEachVoice(lfo1amt = param);
	}
	void processLfo1Dest(float param)
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
	void processLfo2Dest(float param)
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
	void procLfo1Controller(float val)
	{
		int intval = roundToInt(val);
		// off - modwheel - aftertouch - lfo2
		ForEachVoice(lfo1modw = (intval == 1));
		ForEachVoice(lfo1after = (intval == 2));
		ForEachVoice(lfo1vel = (intval == 3));
	}
	void procLfo1ControllerAmt(float val)
	{
		ForEachVoice(lfo1modamt = val);
	}
	void procLfo2Controller(float val)
	{
		int intval = roundToInt(val);
		// off - modwheel - aftertouch - lfo1
		ForEachVoice(lfo2modw = (intval == 1));
		ForEachVoice(lfo2after = (intval == 2));
		ForEachVoice(lfo2vel = (intval == 3));
	}
	void procLfo2ControllerAmt(float val)
	{
		ForEachVoice(lfo2modamt = val);
	}
	void processLfo2Amt(float param)
	{
		ForEachVoice(lfo2amt = param);
	}
	void processOscSpread(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.totalSpread = logsc(param,0.001,0.90);
		}
	}
	void processOsc1PulseWidth(float param)
	{
		//ForEachVoice (osc.osc1pw=linsc(param,0.0,0.95));
		ForEachVoice (osc.osc1pw=param);
	}
	void processOsc2PulseWidth(float param)
	{
		//ForEachVoice(osc.osc2pw = linsc(param,0.0,0.95));
		ForEachVoice (osc.osc2pw=param);
	}
	void processInvertFenv(float param)
	{
		ForEachVoice(invertFenv = roundToInt(param));
	}
	void procFenvLinear(float param)
	{
		ForEachVoice(fenv.setLinear(roundToInt(param)));
	}
	void procEnvMode(float param)
	{
		int intparam = roundToInt(param);
		// Exp env / Lin VCA - Lin env / Lin VCA - Lin env / Exp VCA
		ForEachVoice(env.setLinear(intparam >= 1));
		ForEachVoice(expvca = (intparam >= 2));
	}
	void processOsc2Xmod(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.xmod= param*24;
		}
	}
	void processOsc2SyncLevel(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.syncLevel = 1.0 - param;
		}
	}
	void processOsc1Pitch(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.osc1p = roundToInt(param);
		}
	}
	void processOsc2Pitch(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.osc2p = roundToInt(param);
		}
	}
	void processOsc1Mix(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.o1mx = param;
		}
	}
	void processOsc2Mix(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.o2mx = param;
		}
	}
	void processHPFfreq(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].setHPFfreq(param);
		}
	}
	void processVCADrive(float param)
	{
		ForEachVoice(sqdist.setAmount(param * 0.435));
	}
	void processOsc2FltMod(float param)
	{
		ForEachVoice(osc2FltMod = param*100);
	}
	void processOsc1Det(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.osc1Det = logsc(param,0.001,0.6);
		}
	}
	void processOsc2Det(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.osc2Det = logsc(param,0.001,0.6);
		}
	}

	void processOsc1Wave(float param)
	{
		int intparam = roundToInt(param);
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.osc1Saw = intparam == 1;
			synth.voices[i].osc.osc1Pul = intparam == 2;
			synth.voices[i].osc.osc1Tri = intparam == 3;
			synth.voices[i].osc.osc2modout =
			synth.voices[i].oscmodEnable = intparam != 0;
		}
	}

	void processOsc2Wave(float param)
	{
		int intparam = roundToInt(param);
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].osc.osc2Saw = intparam == 1;
			synth.voices[i].osc.osc2Pul = intparam == 2;
			synth.voices[i].osc.osc2Tri = intparam == 3;
		}
	}
	void processOsc3Wave(float param)
	{
		int intparam = roundToInt(param);
		// off - -1 square - -2 square - -2 pulse - noise
		ForEachVoice(osc.osc3Waveform = intparam);
		//ForEachVoice(osc.noiseEnable = (intparam == 4));
	}
	void processOsc3Mix(float param)
	{
		ForEachVoice(osc.o3mx = param);
	}
	void processCutoff(float param)
	{
		cutoffSmoother.setSteep( linsc(param,0,120));
	//	for(int i = 0 ; i < synth->MAX_VOICES;i++)
	//	{
			//synth->voices[i]->cutoff = logsc(param,60,19000,30);
		//	synth->voices[i]->cutoff = linsc(param,0,120);
	//	}
	}
	inline void processCutoffSmoothed(float param)
	{
		ForEachVoice(cutoff=param);
	}
	void processResonance(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].res = linsc(param,0, 0.991);
		}
	}
	void processResponse(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			//synth.voices[i].flt ;
			synth.voices[i].flt.setResponse(linsc(param,0,1));
		}
	}
	void processOversampling(float param)
	{
		synth.SetOversample(roundToInt(param));
	}
	void processFilterEnvelopeAmt(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			// Linearly scaled to (+/-) 0..70 semitones
			synth.voices[i].fenvamt = linsc(param,0,70);
		}
	}
	void processLoudnessEnvelopeAttack(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].env.setAttack(timesc(param,1,8500));
		}
	}
	void processLoudnessEnvelopeDecay(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].env.setDecay(timesc(param,1,8500));
		}
	}
	void processLoudnessEnvelopeSustainTime(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].env.setSustainTime(timesc(param,1,8500));
			// When time is set to 1.0, sustain time is infinite
			synth.voices[i].env.setAdsr(param > 0.991);
		}
	}
	void processLoudnessEnvelopeRelease(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].env.setRelease(timesc(param,1,8500));
		}
	}
	void processLoudnessEnvelopeSustain(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].env.setSustain(param);
		}
	}
	void processFilterEnvelopeAttack(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].fenv.setAttack(timesc(param,1,8500));
		}
	}
	void processFilterEnvelopeDecay(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].fenv.setDecay(timesc(param,1,8500));
		}
	}
	void processFilterEnvelopeSustainTime(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].fenv.setSustainTime(timesc(param,1,8500));
			// When time is set to 1.0, sustain time is infinite
			synth.voices[i].fenv.setAdsr(param > 0.991);
		}
	}
	void processFilterEnvelopeRelease(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].fenv.setRelease(timesc(param,1,8500));
		}
	}
	void processFilterEnvelopeSustain(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].fenv.setSustain(param);
		}
	}
	void processEnvelopeSpread(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].setEnvSpreadAmt(linsc(param,0.0,1));
		}
	}
	void processLfoSpread(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].setLfoSpreadAmt(linsc(param,0.0,1));
		}
	}
	void processFilterSpread(float param)
	{
for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].FltSpreadAmt = linsc(param,0.0,18);
		}
	}
	void processPortamentoSpread(float param)
	{
		for(int i = 0 ; i < synth.MAX_VOICES;i++)
		{
			synth.voices[i].PortaSpreadAmt = linsc(param,0.0,0.75);
		}
	}
	void processLoudnessSpread(float param)
	{
		ForEachVoice(levelSpreadAmt = linsc(param,0.0,0.67));
	}
	void procOscKeySync(float param)
	{
		ForEachVoice(oscKeySync = roundToInt(param));
	}
	void procEnvRst(float param)
	{
		ForEachVoice(envRst = roundToInt(param));
	}
		 
};
