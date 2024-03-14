/*
	==============================================================================
	This file is part of the MiMi-d synthesizer.

	Copyright 2024 Ricard Wanderlof

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

	// Parameter definitions. This file is pulled into various other
	// files using individual specific macros to extract the relevant
	// information for each case.
	// ParamsEnum.h extracts the symbol (first parameter) in the macros
	// in order to create enums, like the Parameters enum (order is
	// important), 
	// MiMi-d.cpp sets up the process callbacks, and names and symbols
	// of the parameters (and in this case the order is not important).

	// Parameter groups
	PARAMGROUP(PG_MAIN, "Main", "g101_main")
	PARAMGROUP(PG_KEYASGN, "Key Assign", "g102_keyassign")
	PARAMGROUP(PG_VEL, "Velocity Sens.", "g202_velocity")
	PARAMGROUP(PG_LFO1, "LFO1", "g301_mod_lfo1")
	PARAMGROUP(PG_LFO2, "LFO2", "g302_mod_lfo2")
	PARAMGROUP(PG_BEND, "Bend", "g303_bend")
	PARAMGROUP(PG_CONTR, "Controllers", "g503_controllers")
	PARAMGROUP(PG_OSC1, "Oscillator 1", "g401_osc")
	PARAMGROUP(PG_OSC2, "Oscillator 2", "g402_osc")
	PARAMGROUP(PG_OSC_COM, "Osc Common", "g404_osc_com")
	PARAMGROUP(PG_MIXER, "Mixer", "g405_mixer")
	PARAMGROUP(PG_FILTER, "Filter", "g501_filter")
	PARAMGROUP(PG_FILTERCFG, "Filter Config", "g502_filter_config")
	PARAMGROUP(PG_VCA, "VCA", "g503_vca")
	PARAMGROUP(PG_FENV, "Filter Env", "g601_filter_env")
	PARAMGROUP(PG_LENV, "Loudness Env", "g602_loudn_env")
	PARAMGROUP(PG_REL, "Env Release", "g603_release")
	PARAMGROUP(PG_SPREAD, "Spread", "g701_spread")
	PARAMGROUP(PG_DSP, "DSP Control", "g703_dsp")
	PARAMGROUP(PG_MISC, "Miscellaneous", "g704_misc")

	// Scale Points

	PARAMPOINTS(SP_NONE, "") /* No scale points */
	PARAMPOINTS(SP_ONOFF, "OFF", "ON")
	PARAMRANGE(SP_OCTAVE, 1, 5)
	PARAMRANGE(SP_VOICECOUNT, 1, 8)
	PARAMPOINTS(SP_UNISON, "POLY", "UNI")
	PARAMRANGE(SP_BENDRANGE, 0, 12)
	PARAMPOINTS(SP_BENDDEST, " Off ", " Osc1 ", "Osc1+2")
	PARAMPOINTS(SP_LFOWAVE, " Off ", " Saw ", " Fall ", " Tri ", " Rise ", " RSaw ", "Pul ", " Squ ", " RPul ", " S/H ")
	PARAMPOINTS(SP_POLARITY, " Norm ", " Inv ", " Uni ", "InvUni")
	PARAMPOINTS(SP_LFODEST, "Off", "Osc1", "Osc1+2", "Osc2", "PW1", "PW1+2", "PW2", "Filt", "Peak", "Bmod")
	PARAMPOINTS(SP_KEYSYNC, " Free ", "KeySync")
	PARAMRANGE(SP_OSCFREQ, 0, 72)
	PARAMPOINTS(SP_OSCWAVE, " Off ", " Saw ", " Pul ", " Tri ")
	PARAMPOINTS(SP_OSC3WAVE, " Off ", "-1 Squ", "-2 Squ", "-2 Pul", "Noise")
	PARAMPOINTS(SP_FILTERTYPE, " 24 ", "SVF", "SVF+", "SVFN") /* not used */
	PARAMPOINTS(SP_LFOSYNC, " Off ", "Clock", " Key ", "OneShot")
	PARAMPOINTS(SP_LFOCONTR, " None ", " Modw ", "Aftert", " Vel ")
	PARAMPOINTS(SP_CYCRSZ, " CYC ", " RSZ ")
	PARAMPOINTS(SP_ASGNMEM, " Off ", " Mem ")
	PARAMPOINTS(SP_ASGNROB, " Off ", "Rob O", "Rob NL")
	PARAMPOINTS(SP_ASGNRES, " Off ", " Res ")
	PARAMPOINTS(SP_ASGNSTRG, " Off ", " N Off ", " N On+Off ")
	PARAMPOINTS(SP_ENVRST, "FreeRun", "AtkReset")
	PARAMPOINTS(SP_ENVMODE, " Exp ", " Lin ", "LinExp", "LinX5")

	// Parameters

	PARAM(UNUSED_1, PG_MISC, SP_NONE, "Unused 1", "unused_1", 0.0, 1.0, 0.0, procUnused1)
	PARAM(VOLUME, PG_MAIN, SP_NONE, "Volume", "volume", 0.0, 1.0, 0.5, processVolume)
	PARAM(VOICE_COUNT, PG_MAIN, SP_VOICECOUNT, "VoiceCount", "voicecount", 0.0, 1.0, 1.0, setVoiceCount)
	PARAM(TUNE, PG_MAIN, SP_NONE, "Tune", "tune", 0.0, 1.0, 0.5, processTune)
	PARAM(OCTAVE, PG_MAIN, SP_OCTAVE, "Octave", "octave", 0.0, 1.0, 0.5, processOctave)
	PARAM(BENDRANGE, PG_BEND, SP_BENDRANGE, "BendRange", "bendrange", 0.0, 1.0, 0.0, procPitchWheelAmount)
	PARAM(BENDDEST, PG_BEND, SP_BENDDEST, "BendDest", "benddest", 0.0, 1.0, 0.0, procPitchWheelDest)
	PARAM(LFO2FREQ, PG_LFO2, SP_NONE, "Lfo2Frequency", "lfo2frequency", 0.0, 1.0, 0.6, processLfo2Frequency)
	PARAM(VFLTENV, PG_VEL, SP_NONE, "VFltFactor", "vfltfactor", 0.0, 1.0, 0.0, procFltVelocityAmount)
	PARAM(VAMPENV, PG_VEL, SP_NONE, "VAmpFactor", "vampfactor", 0.0, 1.0, 0.0, procAmpVelocityAmount)

	PARAM(PORTAMENTO, PG_MAIN, SP_NONE, "Portamento", "portamento", 0.0, 1.0, 0.0, processPortamento)
	PARAM(UNISON, PG_KEYASGN, SP_UNISON, "Unison", "unison", 0.0, 1.0, 0.0, processUnison)
	PARAM(UDET, PG_SPREAD, SP_NONE, "OscSpread", "oscspread", 0.0, 1.0, 0.2, processOscSpread)
	PARAM(OSC1_DET, PG_OSC1, SP_NONE, "Oscillator1detune", "oscillator1detune", 0.0, 1.0, 0.0, processOsc1Det)
	PARAM(LFO1FREQ, PG_LFO1, SP_NONE, "Lfo1Frequency", "lfo1frequency", 0.0, 1.0, 0.0, processLfo1Frequency)
	PARAM(LFO1WAVE, PG_LFO1, SP_LFOWAVE, "Lfo1Wave", "lfo1wave", 0.0, 1.0, 0.0, processLfo1Wave)
	PARAM(LFO2WAVE, PG_LFO2, SP_LFOWAVE, "Lfo2Wave", "lfo2wave", 0.0, 1.0, 0.0, processLfo2Wave)
	PARAM(LFO1AMT, PG_LFO1, SP_NONE, "Lfo1Amount", "lfo1amount", 0.0, 1.0, 0.0, processLfo1Amt)
	PARAM(LFO2AMT, PG_LFO2, SP_NONE, "Lfo2Amount", "lfo2amount", 0.0, 1.0, 0.0, processLfo2Amt)
	PARAM(LFO1DEST, PG_LFO1, SP_LFODEST, "Lfo1Dest", "lfo1dest", 0.0, 1.0, 0.0, processLfo1Dest)
	PARAM(LFO2DEST, PG_LFO2, SP_LFODEST, "Lfo2Dest", "lfo2dest", 0.0, 1.0, 0.0, processLfo2Dest)
	PARAM(OSC2_DET, PG_OSC2, SP_NONE, "Oscillator2detune", "oscillator2detune", 0.0, 1.0, 0.0, processOsc2Det)
	PARAM(XMOD, PG_OSC_COM, SP_NONE, "Xmod", "xmod", 0.0, 1.0, 0.0, processOsc2Xmod)
	PARAM(OSC1P, PG_OSC1, SP_OSCFREQ, "Osc1Pitch", "osc1pitch", 0.0, 1.0, 0.0, processOsc1Pitch)
	PARAM(OSC2P, PG_OSC2, SP_OSCFREQ, "Osc2Pitch", "osc2pitch", 0.0, 1.0, 0.0, processOsc2Pitch)
	PARAM(OSC1PW, PG_OSC1, SP_NONE, "Osc1PW", "osc1pw", 0.0, 1.0, 0.0, processOsc1PulseWidth)
	PARAM(HPFFREQ, PG_VCA, SP_NONE, "HPFfreq", "hpffreq", 0.0, 1.0, 0.0, processHPFfreq)
	PARAM(OSC1MIX, PG_MIXER, SP_NONE, "Osc1Mix", "osc1mix", 0.0, 1.0, 0.0, processOsc1Mix)
	PARAM(OSC2MIX, PG_MIXER, SP_NONE, "Osc2Mix", "osc2mix", 0.0, 1.0, 1.0, processOsc2Mix)
	PARAM(OSC3MIX, PG_MIXER, SP_NONE, "Osc3Mix", "osc3mix", 0.0, 1.0, 0.0, processOsc3Mix)
	PARAM(FLT_KF, PG_FILTER, SP_NONE, "FilterKeyFollow", "filterkeyfollow", 0.0, 1.0, 0.0, processFilterKeyFollow)
	PARAM(CUTOFF, PG_FILTER, SP_NONE, "Cutoff", "cutoff", 0.0, 1.0, 1.0, processCutoff)
	PARAM(RESONANCE, PG_FILTER, SP_NONE, "Resonance", "resonance", 0.0, 1.0, 0.0, processResonance)
	PARAM(RESPONSE, PG_FILTERCFG, SP_NONE, "Response", "response", 0.0, 1.0, 0.0, processResponse)
	PARAM(OVERSAMPLE, PG_DSP, SP_ONOFF, "Oversample", "oversample", 0.0, 1.0, 1.0, processOversampling)
	PARAM(ENVELOPE_AMT, PG_FILTER, SP_NONE, "FilterEnvAmount", "filterenvamount", 0.0, 1.0, 0.0, processFilterEnvelopeAmt)
	PARAM(LATK, PG_LENV, SP_NONE, "Attack", "attack", 0.0, 1.0, 0.0, processLoudnessEnvelopeAttack)
	PARAM(LDEC, PG_LENV, SP_NONE, "Decay", "decay", 0.0, 1.0, 0.0, processLoudnessEnvelopeDecay)
	PARAM(LSUS, PG_LENV, SP_NONE, "Sustain", "sustain", 0.0, 1.0, 1.0, processLoudnessEnvelopeSustain)
	PARAM(LSUST, PG_LENV, SP_NONE, "SustainTime", "sustaintime", 0.0, 1.0, 1.0, processLoudnessEnvelopeSustainTime)
	PARAM(FATK, PG_FENV, SP_NONE, "FilterAttack", "filterattack", 0.0, 1.0, 0.0, processFilterEnvelopeAttack)
	PARAM(FDEC, PG_FENV, SP_NONE, "FilterDecay", "filterdecay", 0.0, 1.0, 0.0, processFilterEnvelopeDecay)
	PARAM(FSUS, PG_FENV, SP_NONE, "FilterSustain", "filtersustain", 0.0, 1.0, 1.0, processFilterEnvelopeSustain)
	PARAM(FSUST, PG_FENV, SP_NONE, "FilterSustainTime", "filtersustaintime", 0.0, 1.0, 1.0, processFilterEnvelopeSustainTime)
	PARAM(ENVDER, PG_SPREAD, SP_NONE, "EnvelopeSpread", "envelopspread", 0.0, 1.0, 0.0, processEnvelopeSpread)
	PARAM(FILTERDER, PG_SPREAD, SP_NONE, "FilterSpread", "filtespread", 0.0, 1.0, 0.3, processFilterSpread)
	PARAM(PORTADER, PG_SPREAD, SP_NONE, "PortamentoSpread", "portamentospread", 0.0, 1.0, 0.3, processPortamentoSpread)
	PARAM(UNUSED_2, PG_MISC, SP_NONE, "Unused 2", "unused_2", 0.0, 1.0, 0.0, procUnused2)
	PARAM(ECONOMY_MODE, PG_DSP, SP_ONOFF, "EconomyMode", "economymode", 0.0, 1.0, 1.0, procEconomyMode)
	PARAM(LFO1SYNC, PG_LFO1, SP_LFOSYNC, "Lfo1Sync", "lfo1sync", 0.0, 1.0, 0.0, procLfo1Sync)
	PARAM(FENV_INVERT, PG_FILTERCFG, SP_ONOFF, "FenvInvert", "fenvinvert", 0.0, 1.0, 0.0, processInvertFenv)
	PARAM(OSC2PW, PG_OSC2, SP_NONE, "Osc2PW", "osc2pw", 0.0, 1.0, 0.0, processOsc2PulseWidth)
	PARAM(LEVEL_DIF, PG_SPREAD, SP_NONE, "LevelSpread", "levelspread", 0.0, 1.0, 0.3, processLoudnessSpread)
	PARAM(OSC1WAVE, PG_OSC1, SP_OSCWAVE, "Osc1Wave", "osc1wave", 0.0, 1.0, 0.25, processOsc1Wave)
	PARAM(OSC2WAVE, PG_OSC2, SP_OSCWAVE, "Osc2Wave", "osc2wave", 0.0, 1.0, 0.25, processOsc2Wave)
	PARAM(FREL, PG_REL, SP_NONE, "FilterRelease", "filterrelease", 0.0, 1.0, 0.0, processFilterEnvelopeRelease)
	PARAM(LREL, PG_REL, SP_NONE, "Release", "release", 0.0, 1.0, 0.0, processLoudnessEnvelopeRelease)
	PARAM(OSC1FLTMOD, PG_OSC_COM, SP_NONE, "Osc1FilterMod", "osc1filtermod", 0.0, 1.0, 0.0, processOsc1FltMod)

        // ReSet to Zero (lowest) voice (default cyclic)
	PARAM(ASGN_RSZ, PG_KEYASGN, SP_CYCRSZ, "KeyAssignRsz", "keyassignrsz", 0.0, 1.0, 0.0, procKeyAsgnRsz)
        // Prefer assign to voice previously with same note
	PARAM(ASGN_MEM, PG_KEYASGN, SP_ASGNMEM, "KeyAssignMem", "keyassignmem", 0.0, 1.0, 0.0, procKeyAsgnMem)
        // Rob a playing voice if no unplaying available
        // two modes: oldest (O) and next-to-lowest (NL)
	PARAM(ASGN_ROB, PG_KEYASGN, SP_ASGNROB, "KeyAssignRob", "keyassignrob", 0.0, 1.0, 0.0, procKeyAsgnRob)
        // Restore mode: Store notes until voice available
	PARAM(ASGN_RES, PG_KEYASGN, SP_ASGNRES, "KeyAssignRes", "keyassignres", 0.0, 1.0, 0.0, procKeyAsgnRes)
        // Single trig: behavior during rob and restore
	PARAM(ASGN_STRG, PG_KEYASGN, SP_ASGNSTRG, "KeyAssignStrg", "keyassignstrg", 0.0, 1.0, 0.0, procKeyAsgnStrg)

	PARAM(LFO2SYNC, PG_LFO2, SP_LFOSYNC, "Lfo2Sync", "lfo2sync", 0.0, 1.0, 0.0, procLfo2Sync)

	PARAM(LFOSPREAD, PG_SPREAD, SP_NONE, "LfoSpread", "lfospread", 0.0, 1.0, 0.0, processLfoSpread)

	PARAM(PANSPREAD, PG_SPREAD, SP_NONE, "PanSpread", "panspread", 0.0, 1.0, 0.1, processPanSpread)

	PARAM(VCADRIVE, PG_VCA, SP_NONE, "VCADrive", "vcadrive", 0.0, 1.0, 0.0, processVCADrive)

	PARAM(OSCSYNC_LEVEL, PG_OSC_COM, SP_NONE, "SyncLevel", "synclevel", 0.0, 1.0, 0.0, processOsc2SyncLevel)

	PARAM(OSC3WAVE, PG_MIXER, SP_OSC3WAVE, "Osc3Wave", "osc3wave",0.0, 1.0, 0.0, processOsc3Wave)

	PARAM(VEL_SCALE, PG_CONTR, SP_NONE, "VelocityScale", "velocityscale", 0.0, 1.0, 0.0, procVelocityScale)
	PARAM(AT_SCALE, PG_CONTR, SP_NONE, "AfterTouchScale", "aftertouchscale", 0.0, 1.0, 0.0, procAfterTouchScale)

	PARAM(LFO1_AMT_CTRL, PG_LFO1, SP_LFOCONTR, "Lfo1AmtCont", "lfo1amtcont", 0.0, 1.0, 0.0, procLfo1Controller)
	PARAM(LFO2_AMT_CTRL, PG_LFO2, SP_LFOCONTR, "Lfo2AmtCont", "lfo2amtcont", 0.0, 1.0, 0.0, procLfo2Controller)

	PARAM(LFO1_CONTRAMT, PG_LFO1, SP_NONE, "Lfo1ContrAmt", "lfo1contramt", 0.0, 1.0, 0.0, procLfo1ControllerAmt)
	PARAM(LFO2_CONTRAMT, PG_LFO2, SP_NONE, "Lfo2ContrAmt", "lfo2contramt", 0.0, 1.0, 0.0, procLfo2ControllerAmt)

	PARAM(LFO1_POLARITY, PG_LFO1, SP_POLARITY, "Lfo1Polarity", "lfo1polarity", 0.0, 1.0, 0.0, procLfo1Polarity)
	PARAM(LFO2_POLARITY, PG_LFO2, SP_POLARITY, "Lfo2Polarity", "lfo2polarity", 0.0, 1.0, 0.0, procLfo2Polarity)

	PARAM(OSC_KEY_SYNC, PG_OSC_COM, SP_ONOFF, "OscKeySync", "osckeysync", 0.0, 1.0, 0.0, procOscKeySync)
	PARAM(ENV_RST, PG_KEYASGN, SP_ENVRST, "EnvRst", "envrst", 0.0, 1.0, 0.0, procEnvRst)

	PARAM(FENV_LINEAR, PG_FILTERCFG, SP_ONOFF, "FenvLinear", "fenvlinear", 0.0, 1.0, 0.0, procFenvLinear)
	PARAM(ENV_MODE, PG_VCA, SP_ENVMODE, "EnvMode", "envmode", 0.0, 1.0, 0.0, procEnvMode)

// Clean up for potential re-inclusion of file with new definitions
#undef PARAMPOINTS
#undef PARAMRANGE
#undef PARAMGROUP
#undef PARAM
