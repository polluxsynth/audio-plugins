/*
	==============================================================================
	This file is part of the MiMi-d synthesizer.

	Copyright 2024-2025 Ricard Wanderlof

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

	// Define null macros for those that are not defined by the caller

#ifndef PARAMPOINTS
#define PARAMPOINTS(SPID, ...)
#endif
#ifndef PARAMHINTS
#define PARAMHINTS(SPID, HINTS)
#endif
#ifndef PARAMGROUP
#define PARAMGROUP(PGID, NAME, SYMBOL)
#endif
#ifndef PARAM
#define PARAM(PARAMNO, PG, SP, NAME, SYMBOL, MIN, MAX, DEFAULT, SETFUNC)
#endif

	// Parameter definitions. This file is pulled into various other
	// files using individual specific macros to extract the relevant
	// information for each case.
	// ParamsEnum.h extracts the symbol (first parameter) in the macros
	// in order to create enums, like the Parameters enum (order is
	// important), 
	// MiMi-d.cpp sets up the set callbacks, and names and symbols
	// of the parameters (and in this case the order is not important).

	// Parameter groups
	PARAMGROUP(PG_MAIN, "Main", "g101_main")
	PARAMGROUP(PG_KEYASGN, "Key Assign", "g102_keyassign")
	PARAMGROUP(PG_BEND, "Bend", "g303_bend")
	PARAMGROUP(PG_LFO1, "Modulation 1", "g301_mod_lfo1")
	PARAMGROUP(PG_LFO2, "Modulation 2", "g302_mod_lfo2")
	PARAMGROUP(PG_OSC1, "Oscillator 1", "g401_osc1")
	PARAMGROUP(PG_OSC2, "Oscillator 2", "g402_osc2")
	PARAMGROUP(PG_OSC_COM, "Osc Common", "g404_osc_com")
	PARAMGROUP(PG_MIXER, "Mixer", "g405_mixer")
	PARAMGROUP(PG_FILTER, "Filter", "g501_filter")
	PARAMGROUP(PG_FILTERCFG, "Filter Config", "g502_filter_config")
	PARAMGROUP(PG_VCA, "VCA", "g503_vca")
	PARAMGROUP(PG_FENV, "Filter Env", "g601_filter_env")
	PARAMGROUP(PG_LENV, "Loudness Env", "g602_loudn_env")
	PARAMGROUP(PG_CONTR, "Controller Sens.", "g202_controllers")
	PARAMGROUP(PG_SPREAD, "Spread", "g701_spread")
	PARAMGROUP(PG_DSP, "DSP Control", "g703_dsp")
	PARAMGROUP(PG_MISC, "Debugging", "g704_misc")

	// Scale Points

	PARAMPOINTS(SP_NONE, "") /* No scale points - for continuous params */
	PARAMPOINTS(SP_ONOFF, "Off", "On")
	PARAMPOINTS(SP_BENDDEST, " Off ", " Osc1 ", "Osc1+2")
	PARAMPOINTS(SP_ASGNMODE, "Poly", "Mono", "AutoP")
	PARAMPOINTS(SP_CYCRSZ, " Cyc ", " RSZ ")
	PARAMPOINTS(SP_ASGNMEM, " Off ", " Mem ")
	PARAMPOINTS(SP_ASGNROB, " Off ", "Rob O", "Rob NL")
	PARAMPOINTS(SP_ASGNRES, " Off ", " Res ")
	PARAMPOINTS(SP_ASGNMTRG, " All ", "Note On", " None ")
	PARAMPOINTS(SP_ENVRST, "FreeRun", "KeyReset")
	PARAMPOINTS(SP_EXPLIN, "Exp", "Lin")
	PARAMPOINTS(SP_LFOWAVE, " Off ", " Saw ", " Fall ", " Tri ", " Rise ", " RSaw ", "Pul ", " Squ ", " RPul ", " S/H ")
	PARAMPOINTS(SP_LFOSYNC, " Off ", "Clock", " Key ", "OneShot")
	PARAMPOINTS(SP_LFOCONTR, " None ", " Modw ", "Aftert", " Vel ")
	PARAMPOINTS(SP_LFODEST, "Off", "Osc1", "Osc1+2", "Osc2", "SH1", "SH1+2", "SH2", "Cutoff", "Res", "Osc2Filt")
	PARAMPOINTS(SP_POLARITY, " Norm ", " Inv ", " Uni ", "InvUni")
	PARAMPOINTS(SP_OSCWAVE, " Off ", " Saw ", " Pul ", " Tri ")
	PARAMPOINTS(SP_KEYSYNC, "FreeRun", "KeySync")
	PARAMPOINTS(SP_OSC3WAVE, " Off ", "-1 Squ", "-2 Squ", "-2 Pul", "Noise")
	PARAMPOINTS(SP_ENVMODE, "Exp/Lin ", "Lin/Lin", "Lin/Exp")

	PARAMHINTS(SP_INTS, kParameterIsInteger)

	PARAMHINTS(SP_HIDDEN, kParameterIsHidden)

	// Parameters

	// Main
	PARAM(VOLUME, PG_MAIN, SP_NONE, "Volume", "volume", 0, 10, 5, setVolume)
	PARAM(PORTAMENTO, PG_MAIN, SP_NONE, "Portamento", "portamento", 0, 10, 0, setPortamento)
	PARAM(TUNE, PG_MAIN, SP_NONE, "Tune", "tune", -1.0, 1.0, 0.0, setTune)
	PARAM(OCTAVE, PG_MAIN, SP_INTS, "Octave", "octave", -2, 2, 0, setOctave)

	// Key assignment #1 (general)
	PARAM(VOICE_COUNT, PG_KEYASGN, SP_INTS, "VoiceCount", "voicecount", 1, 32, 6, setVoiceCount)
	PARAM(ASGN_MODE, PG_KEYASGN, SP_ASGNMODE, "Assign Mode", "keyasgnmode", 0, SP_MAX, 0, setKeyAsgnMode)
	// Envelope reset
	PARAM(ENV_RST, PG_KEYASGN, SP_ENVRST, "Envelope Attack", "envrst", 0, SP_MAX, 0, setEnvRst)
        // Single trig: behavior during rob and restore
	PARAM(ASGN_MTRG, PG_KEYASGN, SP_ASGNMTRG, "Env Retrig", "keyassignmtrg", 0, SP_MAX, 0, setKeyAsgnStrg)

	// Key assignment #2 (modes)
        // ReSet to Zero (lowest) voice (default cyclic)
	PARAM(ASGN_RSZ, PG_KEYASGN, SP_CYCRSZ, "Assign Order", "keyassignrsz", 0, SP_MAX, 0, setKeyAsgnRsz)
        // Prefer assign to voice previously with same note
	PARAM(ASGN_MEM, PG_KEYASGN, SP_ASGNMEM, "Assign Memory", "keyassignmem", 0, SP_MAX, 1, setKeyAsgnMem)
        // Rob a playing voice if no unplaying available
        // two modes: oldest (O) and next-to-lowest (NL)
	PARAM(ASGN_ROB, PG_KEYASGN, SP_ASGNROB, "Voice Rob", "keyassignrob", 0, SP_MAX, 2, setKeyAsgnRob)
        // Restore mode: Store notes until voice available
	PARAM(ASGN_RES, PG_KEYASGN, SP_ASGNRES, "Voice Restore", "keyassignres", 0, SP_MAX, 0, setKeyAsgnRes)

	// Bend
	PARAM(BENDRANGE, PG_BEND, SP_INTS, "Range", "bendrange", 0, 12, 0, setPitchWheelAmount)
	PARAM(BENDDEST, PG_BEND, SP_BENDDEST, "Dest", "benddest", 0, SP_MAX, 0, setPitchWheelDest)

	// LFO1 #1 (main: freq, wave, basic amount)
	PARAM(LFO1FREQ, PG_LFO1, SP_NONE, "Rate", "lfo1rate", 0, 10, 0, setLfo1Frequency)
	PARAM(LFO1WAVE, PG_LFO1, SP_LFOWAVE, "Wave", "lfo1wave", 0, SP_MAX, 0, setLfo1Wave)
	PARAM(LFO1AMT, PG_LFO1, SP_NONE, "Initial Amount", "lfo1amount", 0, 10, 0, setLfo1Amt)
	PARAM(LFO1DEST, PG_LFO1, SP_LFODEST, "Dest", "lfo1dest", 0, SP_MAX, 0, setLfo1Dest)
	// LFO1 #2 (secondary: sync, controllers, polarity)
	PARAM(LFO1_CONTRAMT, PG_LFO1, SP_NONE, "Controller Amount", "lfo1contramt", 0, 10, 0, setLfo1ControllerAmt)
	PARAM(LFO1_AMT_CTRL, PG_LFO1, SP_LFOCONTR, "Controller", "lfo1amtcont", 0, SP_MAX, 0, setLfo1Controller)
	PARAM(LFO1_POLARITY, PG_LFO1, SP_POLARITY, "Polarity", "lfo1polarity", 0, SP_MAX, 0, setLfo1Polarity)
	PARAM(LFO1SYNC, PG_LFO1, SP_LFOSYNC, "Sync", "lfo1sync", 0, SP_MAX, 0, setLfo1Sync)

	// LFO2 #1 (main: freq, wave, basic amount)
	PARAM(LFO2FREQ, PG_LFO2, SP_NONE, "Rate", "lfo2frequency", 0, 10, 6, setLfo2Frequency)
	PARAM(LFO2WAVE, PG_LFO2, SP_LFOWAVE, "Wave", "lfo2wave", 0, SP_MAX, 0, setLfo2Wave)
	PARAM(LFO2AMT, PG_LFO2, SP_NONE, "Initial Amount", "lfo2amount", 0, 10, 0, setLfo2Amt)
	PARAM(LFO2DEST, PG_LFO2, SP_LFODEST, "Dest", "lfo2dest", 0, SP_MAX, 0, setLfo2Dest)
	// LFO2 #2 (secondary: sync, controllers, polarity)
	PARAM(LFO2_CONTRAMT, PG_LFO2, SP_NONE, "Controller Amount", "lfo2contramt", 0, 10, 0, setLfo2ControllerAmt)
	PARAM(LFO2_AMT_CTRL, PG_LFO2, SP_LFOCONTR, "Controller", "lfo2amtcont", 0, SP_MAX, 0, setLfo2Controller)
	PARAM(LFO2_POLARITY, PG_LFO2, SP_POLARITY, "Polarity", "lfo2polarity", 0, SP_MAX, 0, setLfo2Polarity)
	PARAM(LFO2SYNC, PG_LFO2, SP_LFOSYNC, "Sync", "lfo2sync", 0, SP_MAX, 0, setLfo2Sync)

	// Osc 1
	PARAM(OSC1_DET, PG_OSC1, SP_NONE, "Detune", "osc1detune", -1.0, 1.0, 0.0, setOsc1Det)
	PARAM(OSC1P, PG_OSC1, SP_INTS, "Pitch", "osc1pitch", 0, 72, 24, setOsc1Pitch)
	PARAM(OSC1SH, PG_OSC1, SP_NONE, "Shape", "osc1pw", 0, 10, 0, setOsc1Shape)
	PARAM(OSC1WAVE, PG_OSC1, SP_OSCWAVE, "Wave", "osc1wave", 0, SP_MAX, 0, setOsc1Wave)

	// Osc 2
	PARAM(OSC2_DET, PG_OSC2, SP_NONE, "Detune", "osc2detune", -1.0, 1.0, 0.0, setOsc2Det)
	PARAM(OSC2P, PG_OSC2, SP_INTS, "Pitch", "osc2pitch", 0, 72, 24, setOsc2Pitch)
	PARAM(OSC2SH, PG_OSC2, SP_NONE, "Shape", "osc2pw", 0, 10, 0, setOsc2Shape)
	PARAM(OSC2WAVE, PG_OSC2, SP_OSCWAVE, "Wave", "osc2wave", 0, SP_MAX, 1, setOsc2Wave)

	// Osc common (osc modulation and key sync)
	PARAM(XMOD, PG_OSC_COM, SP_NONE, "Xmod", "xmod", 0, 10, 0, setOsc2Xmod)
	PARAM(OSC2FLTMOD, PG_OSC_COM, SP_NONE, "Osc2FilterMod", "osc2filtermod", 0, 10, 0, setOsc2FltMod)
	PARAM(OSCSYNC_LEVEL, PG_OSC_COM, SP_NONE, "SyncLevel", "synclevel", 0, 10, 0, setOsc2SyncLevel)
	PARAM(OSC_KEY_SYNC, PG_OSC_COM, SP_KEYSYNC, "Waveform Reset", "osckeysync", 0, SP_MAX, 0, setOscKeySync)

	// Mixer and sub osc
	PARAM(OSC1MIX, PG_MIXER, SP_NONE, "Osc1Mix", "osc1mix", 0, 10, 0, setOsc1Mix)
	PARAM(OSC2MIX, PG_MIXER, SP_NONE, "Osc2Mix", "osc2mix", 0, 10, 10, setOsc2Mix)
	PARAM(OSC3MIX, PG_MIXER, SP_NONE, "Osc2Sub Mix", "osc2submix", 0, 10, 0, setOsc2SubMix)
	PARAM(OSC3WAVE, PG_MIXER, SP_OSC3WAVE, "Osc2Sub Wave", "osc2subwave", 0, SP_MAX, 0, setOsc2SubWave)

	// Filter configuration
	PARAM(RESPONSE, PG_FILTERCFG, SP_NONE, "Pole Count", "response", 1, 4, 4, setResponse)
	PARAM(FENV_INVERT, PG_FILTERCFG, SP_ONOFF, "Env Invert", "fenvpolarity", 0, SP_MAX, 0, setInvertFenv)
	PARAM(FENV_LINEAR, PG_FILTERCFG, SP_EXPLIN, "Env Shape", "fenvmode", 0, SP_MAX, 0, setFenvLinear)

	// Filter
	PARAM(CUTOFF, PG_FILTER, SP_NONE, "Cutoff", "cutoff", 0, 10, 10, setCutoff)
	PARAM(RESONANCE, PG_FILTER, SP_NONE, "Resonance", "resonance", 0, 10, 0, setResonance)
	PARAM(FLT_KF, PG_FILTER, SP_NONE, "KeyTrack", "keytrack", 0.0, 1.0, 0.0, setFilterKeyFollow)
	PARAM(ENVELOPE_AMT, PG_FILTER, SP_NONE, "EnvAmount", "filterenvamount", 0, 10, 0, setFilterEnvelopeAmt)

	// FENV
	PARAM(FATK, PG_FENV, SP_NONE, "Filter Attack Time", "filterattack", 0, 10, 0, setFilterEnvelopeAttack)
	PARAM(FDEC, PG_FENV, SP_NONE, "Filter Decay Time", "filterdecay", 0, 10, 0, setFilterEnvelopeDecay)
	PARAM(FSUS, PG_FENV, SP_NONE, "Filter Sustain Level", "filtersustain", 0, 10, 10, setFilterEnvelopeSustain)
	PARAM(FSUST, PG_FENV, SP_NONE, "Filter Sustain Time", "filtersustaintime", 0, 10, 10, setFilterEnvelopeSustainTime)
	PARAM(FREL, PG_FENV, SP_NONE, "Filter Release", "filterrelease", 0, 10, 0, setFilterEnvelopeRelease)

	// LENV
	PARAM(LATK, PG_LENV, SP_NONE, "Loudn Attack Time", "attack", 0, 10, 0, setLoudnessEnvelopeAttack)
	PARAM(LDEC, PG_LENV, SP_NONE, "Loudn Decay Time", "decay", 0, 10, 0, setLoudnessEnvelopeDecay)
	PARAM(LSUS, PG_LENV, SP_NONE, "Loudn Sustain Level", "sustain", 0, 10, 10, setLoudnessEnvelopeSustain)
	PARAM(LSUST, PG_LENV, SP_NONE, "Loudn Sustain Time", "sustaintime", 0, 10, 10, setLoudnessEnvelopeSustainTime)
	PARAM(LREL, PG_LENV, SP_NONE, "Loudn Release", "release", 0, 10, 0, setLoudnessEnvelopeRelease)

	// Env release (moved to envelope pages)

	// VCA (+ Loudness envelope control)
	PARAM(HPFFREQ, PG_VCA, SP_NONE, "HPF Freq", "hpffreq", 0, 10, 0, setHPFfreq)
	PARAM(VCADRIVE, PG_VCA, SP_NONE, "VCA Drive", "vcadrive", 0, 10, 0, setVCADrive)
	PARAM(ENV_MODE, PG_VCA, SP_ENVMODE, "Env/VCA Mode", "envmode", 0, SP_MAX, 0, setEnvMode)

	// Controller sensitivity and control (velocity and aftertouch)
	PARAM(VEL_SCALE, PG_CONTR, SP_NONE, "Velocity Scale", "velocityscale", 0, 10, 5, setVelocityScale)
	PARAM(AT_SCALE, PG_CONTR, SP_NONE, "AfterTouch Scale", "aftertouchscale", 0, 10, 5, setAfterTouchScale)
	PARAM(VFLTENV, PG_CONTR, SP_NONE, "FilterEnv Velocity", "vfltfactor", 0, 10, 0, setFltVelocityAmount)
	PARAM(VAMPENV, PG_CONTR, SP_NONE, "Amp Velocity", "vampfactor", 0, 10, 0, setAmpVelocityAmount)

	// Spread control
	// #1 (freqs and levels)
	PARAM(UDET, PG_SPREAD, SP_NONE, "OscSpread", "oscspread", 0, 10, 2, setOscSpread)
	PARAM(FILTERDER, PG_SPREAD, SP_NONE, "FilterSpread", "filterspread", 0, 10, 3, setFilterSpread)
	PARAM(LEVEL_DIF, PG_SPREAD, SP_NONE, "LevelSpread", "levelspread", 0, 10, 3, setLoudnessSpread)
	PARAM(PANSPREAD, PG_SPREAD, SP_NONE, "PanSpread", "panspread", 0, 10, 1, setPanSpread)
	// #2 (times and speeds)
	PARAM(ENVDER, PG_SPREAD, SP_NONE, "EnvelopeSpread", "envelopspread", 0, 10, 0, setEnvelopeSpread)
	PARAM(PORTADER, PG_SPREAD, SP_NONE, "PortamentoSpread", "portamentospread", 0, 10, 3, setPortamentoSpread)
	PARAM(LFOSPREAD, PG_SPREAD, SP_NONE, "LfoSpread", "lfospread", 0, 10, 0, setLfoSpread)

	// DSP control
	PARAM(OVERSAMPLE, PG_DSP, SP_ONOFF, "Oversample", "oversample", 0, SP_MAX, 1, setOversampling)
	PARAM(ECONOMY_MODE, PG_DSP, SP_ONOFF, "Economy Mode", "economymode", 0, SP_MAX, 1, setEconomyMode)

	// Misc/Debug
	PARAM(UNUSED_1, PG_MISC, SP_HIDDEN, "Debug 1", "unused_1", 0, 1.0, 0, procUnused1)
	PARAM(UNUSED_2, PG_MISC, SP_HIDDEN, "Debug 2", "unused_2", 0, 1.0, 0, procUnused2)

// Clean up for potential re-inclusion of file with new definitions
#undef PARAMPOINTS
#undef PARAMHINTS
#undef PARAMGROUP
#undef PARAM
