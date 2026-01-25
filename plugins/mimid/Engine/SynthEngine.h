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

#include "Voice.h"
#include "Motherboard.h"
#include "Params.h"
#include "ParamSmoother.h"

#define ForEachVoice(expr) \
	for (int i = 0; i < synth.MAX_VOICES; i++) {\
			synth.voices[i].expr;\
		}

class SynthEngine
{
private:
	Motherboard synth;
	ParamSmoother cutoffSmoother;
	ParamSmoother pitchWheelSmoother;
	ParamSmoother modWheelSmoother;
	float sampleRate;
	float atscale;
	float velscale;
	float totalTune; // Corresponding to tune parameter
	float octaveTune; // Octave setting converted to tune (semitones)
	// TODO Remove unused1,2:
	float unused1, unused2;

public:
	SynthEngine():
	cutoffSmoother(),
	pitchWheelSmoother(),
	modWheelSmoother()
	{
		atscale = 1;
		velscale = 1;
		totalTune = octaveTune = 0;
	}
	~SynthEngine()
	{
	}
	void setBpm(float bpm)
	{
		for (int i = 0; i < synth.MAX_VOICES; i++) {
			synth.voices[i].lfo1.setBpm(bpm);
			synth.voices[i].lfo2.setBpm(bpm);
			synth.voices[i].lfo3.setBpm(bpm);
		}
	}
	void setPlayHead(float retrPos)
	{
		for (int i = 0; i < synth.MAX_VOICES; i++) {
			synth.voices[i].lfo1.hostSyncRetrigger(retrPos);
			synth.voices[i].lfo2.hostSyncRetrigger(retrPos);
			synth.voices[i].lfo3.hostSyncRetrigger(retrPos);
		}
	}
	void setSampleRate(float sr)
	{
		sampleRate = sr;
		cutoffSmoother.setSampleRate(sr);
		pitchWheelSmoother.setSampleRate(sr);
		modWheelSmoother.setSampleRate(sr);
		synth.setSampleRate(sr);
	}
	void processSample(float *left,float *right)
	{
		processCutoffSmoothed(cutoffSmoother.smoothStep());
		procPitchWheelSmoothed(pitchWheelSmoother.smoothStep());
		procModWheelSmoothed(modWheelSmoother.smoothStep());

		synth.processSample(left, right);
	}
	void allNotesOff()
	{
		for (int i = 0; i < 128; i++)
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
		for (int i = 0; i < Motherboard::MAX_VOICES; i++) {
			synth.voices[i].lfo1.setClockSync(intval == 1);
			synth.voices[i].lfo1.setKeySync(intval >= 2 && intval <= 3);
			synth.voices[i].lfo1.setOneShot(intval == 3);
		}
	}
	void setLfo2Sync(float val)
	{
		// Off - Tempo - Key - Oneshot
		int intval = roundToInt(val);
		for (int i = 0; i < Motherboard::MAX_VOICES; i++) {
			synth.voices[i].lfo2.setClockSync(intval == 1);
			synth.voices[i].lfo2.setKeySync(intval >= 2 && intval <= 3);
			synth.voices[i].lfo2.setOneShot(intval == 3);
		}
	}
	void setLfo1Polarity(float val)
	{
		// Normal - Invert - Unipolar - Unipolar+Invert
		int intval = roundToInt(val) & 3;

		ForEachVoice(lfo1.setPolarity(intval));
	}
	void setLfo2Polarity(float val)
	{
		// Normal - Invert - Unipolar - Unipolar+Invert
		int intval = roundToInt(val) & 3;

		ForEachVoice(lfo2.setPolarity(intval));
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
		// Poly - Mono - Mono+Auto [Portamento] - Dual
		int intval = roundToInt(param);
		synth.voiceAlloc.uni = intval == 1 || intval == 2;
		synth.voiceAlloc.alwaysPorta = intval != 2;
		synth.voiceAlloc.dual = intval == 3;
	}
	void setUnisonPanAmt(float param)
	{
		synth.setUnisonPanAmt(param * 0.1f);
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
		unused1 = val;
		ForEachVoice(unused1 = val);
		ForEachVoice(osc.unused1 = val);
		ForEachVoice(flt.unused1 = val);
		ForEachVoice(env.unused1 = val);
		ForEachVoice(fenv.unused1 = val);
	}
	// TODO: Remove
	void procUnused2(float val)
	{
		unused2 = val;
		ForEachVoice(unused2 = val);
		ForEachVoice(osc.unused2 = val);
		ForEachVoice(flt.unused2 = val);
		ForEachVoice(env.unused2 = val);
		ForEachVoice(fenv.unused2 = val);
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
		synth.voiceAlloc.setAfterTouch(val);
		for (int i = 0; i < synth.MAX_VOICES; i++)
			synth.voices[i].afterTouchSmoother.setSteep(val);
	}
	void procAfterTouch(int note, float val)
	{
		val = powf(val, atscale);
		synth.voiceAlloc.setAfterTouch(note, val);
		for (int i = 0; i < synth.MAX_VOICES; i++) {
			if (note == synth.voices[i].midiIndx)
				// TODO: Should we only do this for voices
				// that are actually playing?
				// (OTOH: If released, there's likely not
				// going to be any aftertouch is there?).
				synth.voices[i].afterTouchSmoother.setSteep(val);
		}
	}
	void procAfterTouchSmoothed(float val)
	{
		ForEachVoice(aftert = val);
	}
	void setLfo2Frequency(float val)
	{
		for (int i = 0; i < synth.MAX_VOICES; i++) {
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
		for (int i = 0; i < synth.MAX_VOICES; i++)
			synth.voices[i].setPwRoute(synth.voices[i].pwroute, intparam);
	}
	void setModWheelAmount(float param)
	{
		param *= 0.1f; // 0..10 -> 0..1
		ForEachVoice(modWheelAmt = param);
	}
	void setModWheelDest(float param)
	{
		int intparam = roundToInt(param);
		// off, osc1, osc1+2, osc2, pw1, pw1+2, pw2, filt, res, bmod
		// 0    1     2       3     4    5      6    7     8    9
		ForEachVoice(setModWheelRoute(intparam));
	}
	void setAfterTouchAmount(float param)
	{
		param *= 0.1f; // 0..10 -> 0..1
		ForEachVoice(afterTouchAmt = param);
	}
	void setAfterTouchDest(float param)
	{
		int intparam = roundToInt(param);
		// off, osc1, osc1+2, osc2, pw1, pw1+2, pw2, filt, res, bmod
		// 0    1     2       3     4    5      6    7     8    9
		ForEachVoice(setAfterTouchRoute(intparam));
	}
	void setPanSpread(float param)
	{
		synth.setPanSpreadAmt(param * 0.1f);
	}
	void setTuneAndOctave()
	{
		float tune = totalTune + octaveTune;

		ForEachVoice(osc.oct_tune = tune);
		ForEachVoice(filtertune = tune);
	}
	void setTune(float param)
	{
		totalTune = param;
		setTuneAndOctave();
	}
	void setOctave(float param)
	{
		// Add 2 before rounding to avoid problems around zero
		octaveTune = (roundToInt(param + 2.0f) - 2) * 12;
		setTuneAndOctave();
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
		synth.volume = linsc(param, 0, 0.30f);
	}
	void setLfo1Frequency(float param)
	{
		for (int i = 0; i < synth.MAX_VOICES; i++) {
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
		param *= 0.1f; // 0..10 -> 0..1
		param *= param; // square to get better low end resolution
		ForEachVoice(lfo1amt = param);
	}
	void setLfo1Dest(float param)
	{
		int intparam = roundToInt(param);
		// off, osc1, osc1+2, osc2, pw1, pw1+2, pw2, filt, res, bmod
		// 0    1     2       3     4    5      6    7     8    9
		ForEachVoice(setMod1Route(intparam));
	}
	void setLfo2Dest(float param)
	{
		int intparam = roundToInt(param);
		// off, osc1, osc1+2, osc2, pw1, pw1+2, pw2, filt, res, bmod
		// 0    1     2       3     4    5      6    7     8    9
		ForEachVoice(setMod2Route(intparam));
	}
	void setLfo1Controller(float val)
	{
		int intval = roundToInt(val);
		// off - modwheel - aftertouch - vel
		for (int i = 0; i < synth.MAX_VOICES; i++)
			synth.voices[i].setModController(&synth.voices[i].lfo1controller, intval);
	}
	void setLfo1ControllerAmt(float val)
	{
		ForEachVoice(lfo1contramt = val * 0.1f);
	}
	void setLfo2Controller(float val)
	{
		int intval = roundToInt(val);
		// off - modwheel - aftertouch - vel
		for (int i = 0; i < synth.MAX_VOICES; i++)
			synth.voices[i].setModController(&synth.voices[i].lfo2controller, intval);
	}
	void setLfo2ControllerAmt(float val)
	{
		ForEachVoice(lfo2contramt = val * 0.1f);
	}
	void setLfo2Amt(float param)
	{
		param *= 0.1f; // 0..10 -> 0..1
		param *= param; // square to get better low end resolution
		ForEachVoice(lfo2amt = param);
	}
	void setLfo3Frequency(float param)
	{
		for (int i = 0; i < synth.MAX_VOICES; i++) {
			synth.voices[i].lfo3.setRawFrequency(param);
			synth.voices[i].lfo3.setFrequency(logsc(param, 0, 100, 240));
		}
	}
	void setLfo3Shape(float param)
	{
		// TODO: put scaling in setWaveForm ?
		ForEachVoice(lfo3.setShape(param * 0.1f));
	}
	void setLfo3Amt(float param)
	{
		param *= 0.1f; // 0..10 -> 0..1
		param *= param; // square to get better low end resolution
		ForEachVoice(lfo3amt = param);
	}
	void setLfo3Dest(float param)
	{
		int intparam = roundToInt(param);
		// off, osc1, osc1+2, osc2, pw1, pw1+2, pw2, filt, res, bmod
		// 0    1     2       3     4    5      6    7     8    9
		ForEachVoice(setMod3Route(intparam));
	}
	void setLfo3Controller(float val)
	{
		int intval = roundToInt(val);
		// off - modwheel - aftertouch - vel
		for (int i = 0; i < synth.MAX_VOICES; i++)
			synth.voices[i].setModController(&synth.voices[i].lfo3controller, intval);
	}
	void setLfo3ControllerAmt(float val)
	{
		ForEachVoice(lfo3contramt = val * 0.1f);
	}
	void setLfo3Sync(float val)
	{
		// Off - Tempo - Key - Oneshot
		int intval = roundToInt(val);
		for (int i = 0; i < Motherboard::MAX_VOICES; i++) {
			synth.voices[i].lfo3.setClockSync(intval == 1);
			synth.voices[i].lfo3.setKeySync(intval >= 2 && intval <= 3);
			synth.voices[i].lfo3.setOneShot(intval == 3);
		}
	}
	void setLfo3Polarity(float val)
	{
		// Normal - Invert - Unipolar - Unipolar+Invert
		int intval = roundToInt(val) & 3;

		ForEachVoice(lfo3.setPolarity(intval));
	}
	void setOscSpread(float param)
	{
		float totalSpread = logsc(param, 0.001f, 0.90f);

		ForEachVoice(osc.setOscSpread(totalSpread));
	}
	void setOsc1Shape(float param)
	{
		ForEachVoice (osc.oscparams.osc1sh = param * 0.1f);
	}
	void setOsc2Shape(float param)
	{
		ForEachVoice (osc.oscparams.osc2sh = param * 0.1f);
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
		ForEachVoice(osc.oscparams.osc1p = roundToInt(param));
	}
	void setOsc2Pitch(float param)
	{
		ForEachVoice(osc.oscparams.osc2p = roundToInt(param));
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
		// linsc is geared for 0..10. so we multiply param by 10
		// as the displayed parameter value is 0..1.
		ForEachVoice(osc.oscparams.osc1Det = linsc(param * 10, 0, 1.0f));
	}
	void setOsc2Det(float param)
	{
		ForEachVoice(osc.oscparams.osc2Det = linsc(param * 10, 0, 1.0f));
	}

	void setOsc1Wave(float param)
	{
		int intparam = roundToInt(param);
		for (int i = 0; i < synth.MAX_VOICES; i++)
			synth.voices[i].osc.oscparams.osc1Wave = intparam;
	}

	void setOsc2Wave(float param)
	{
		int intparam = roundToInt(param);
		for (int i = 0; i < synth.MAX_VOICES; i++) {
			synth.voices[i].osc.oscparams.osc2Wave = intparam;
			synth.voices[i].osc.osc2modout =
				synth.voices[i].oscmodEnable = intparam != 0;
		}
	}
	void setOsc2SubWave(float param)
	{
		int intparam = roundToInt(param);
		// off - -1 square - -2 square - -2 pulse - noise
		ForEachVoice(osc.oscparams.osc2SubWaveform = intparam);
	}
	void setOsc2SubMix(float param)
	{
		ForEachVoice(osc.o2submx = param * 0.1f);
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
		synth.setOversample(roundToInt(param));
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
	void setLoudnessEnvelopeHold(float param)
	{
		ForEachVoice(env.setHold(timesc(param, 0.01f, 12500)));
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
	void setFilterEnvelopeHold(float param)
	{
		ForEachVoice(fenv.setHold(timesc(param, 0.01f, 12500)));
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
		float FltSpreadAmt = linsc(param, 0, 18);
		for (int i = 0; i < synth.MAX_VOICES; i++)
			synth.voices[i].FltSpreadAmt =
				FltSpreadAmt * synth.voices[i].FltSpread;
	}
	void setPortamentoSpread(float param)
	{
		float PortaSpreadAmt = linsc(param, 0.0f, 0.75f);
		for (int i = 0; i < synth.MAX_VOICES; i++)
			synth.voices[i].PortaSpreadAmt =
				1 + PortaSpreadAmt * synth.voices[i].PortaSpread;
	}
	void setLoudnessSpread(float param)
	{
		float levelSpreadAmt = linsc(param, 0.0f, 0.67f);
		for (int i = 0; i < synth.MAX_VOICES; i++)
			synth.voices[i].levelSpreadAmt =
				1 - levelSpreadAmt * synth.voices[i].levelSpread;
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
