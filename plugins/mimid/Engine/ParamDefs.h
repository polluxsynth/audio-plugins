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
	// TODO: Remove PG_UNUSED (used by PARAM_NULL)
	PARAMGROUP(PG_NOTUSED, "Notused", "g000_notused")
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

	// Parameters

	PARAM(UNUSED_1, PG_MISC, SP_NONE, "Unused 1", "unused_1", 0.0, 1.0, 0.0, procUnused1)
	PARAM(VOLUME, PG_MAIN, SP_NONE, "Volume", "volume", 0.0, 1.0, 0.5, processVolume)
	PARAM(VOICE_COUNT, PG_MAIN, SP_VOICECOUNT, "VoiceCount", "voicecount", 0.0, 1.0, 1.0, setVoiceCount)
	PARAM(TUNE, PG_MAIN, SP_NONE, "Tune", "tune", 0.0, 1.0, 0.5, processTune)
	PARAM(OCTAVE, PG_MAIN, SP_OCTAVE, "Octave", "octave", 0.0, 1.0, 0.5, processOctave)
	PARAM(BENDRANGE, PG_BEND, SP_BENDRANGE, "BendRange", "bendrange", 0.0, 1.0, 0.0, procPitchWheelAmount)
	PARAM(BENDDEST, PG_BEND, SP_BENDDEST, "BendDest", "benddest", 0.0, 1.0, 0.0, procPitchWheelDest)
	PARAM_NULL(LEGATOMODE_NOTUSED, "LegatoMode_NOTUSED", "legatomode_unused")
	PARAM(LFO2FREQ, PG_LFO2, SP_NONE, "Lfo2Frequency", "lfo2frequency", 0.0, 1.0, 0.6, processLfo2Frequency)
	PARAM(VFLTENV, PG_VEL, SP_NONE, "VFltFactor", "vfltfactor", 0.0, 1.0, 0.0, procFltVelocityAmount)
	PARAM(VAMPENV, PG_VEL, SP_NONE, "VAmpFactor", "vampfactor", 0.0, 1.0, 0.0, procAmpVelocityAmount)

	PARAM_NULL(ASPLAYEDALLOCATION_NOTUSED, "AsPlayedAllocation_NOTUSED", "asplayedallocation_notused")
	PARAM(PORTAMENTO, PG_MAIN, SP_NONE, "Portamento", "portamento", 0.0, 1.0, 0.0, processPortamento)
	PARAM(UNISON, PG_KEYASGN, SP_UNISON, "Unison", "unison", 0.0, 1.0, 0.0, processUnison)
	PARAM(UDET, PG_SPREAD, SP_NONE, "OscSpread", "oscspread", 0.0, 1.0, 0.2, processOscSpread)
	PARAM(OSC1_DET, PG_OSC1, SP_NONE, "Oscillator1detune", "oscillator1detune", 0.0, 1.0, 0.0, processOsc1Det)
	PARAM(LFO1FREQ, PG_LFO1, SP_NONE, "Lfo1Frequency", "lfo1frequency", 0.0, 1.0, 0.0, processLfo1Frequency)
	PARAM(LFO1WAVE, PG_LFO1, SP_LFOWAVE, "Lfo1Wave", "lfo1wave", 0.0, 1.0, 0.0, processLfo1Wave)
	PARAM(LFO2WAVE, PG_LFO2, SP_LFOWAVE, "Lfo2Wave", "lfo2wave", 0.0, 1.0, 0.0, processLfo2Wave)
	PARAM_NULL(LFOSHWAVE_UNUSED, "LfoSampleHoldWave_NOTUSED", "lfosampleholdwave_notused")
	PARAM(LFO1AMT, PG_LFO1, SP_NONE, "Lfo1Amount", "lfo1amount", 0.0, 1.0, 0.0, processLfo1Amt)
	PARAM(LFO2AMT, PG_LFO2, SP_NONE, "Lfo2Amount", "lfo2amount", 0.0, 1.0, 0.0, processLfo2Amt)
	PARAM(LFO1DEST, PG_LFO1, SP_LFODEST, "Lfo1Dest", "lfo1dest", 0.0, 1.0, 0.0, processLfo1Dest)
	PARAM(LFO2DEST, PG_LFO2, SP_LFODEST, "Lfo2Dest", "lfo2dest", 0.0, 1.0, 0.0, processLfo2Dest)
	PARAM_NULL(LFOFILTER_UNUSED, "LfoFilter_NOTUSED", "lfofilter_notused")
	PARAM_NULL(LFOPW1_UNUSED, "LfoPw1_NOTUSED", "lfopw1_notused")
	PARAM_NULL(LFOPW2_UNUSED, "LfoPw2_NOTUSED", "lfopw2_notused")
	PARAM(OSC2_DET, PG_OSC2, SP_NONE, "Oscillator2detune", "oscillator2detune", 0.0, 1.0, 0.0, processOsc2Det)
	PARAM(XMOD, PG_OSC_COM, SP_NONE, "Xmod", "xmod", 0.0, 1.0, 0.0, processOsc2Xmod)
	PARAM(OSC1P, PG_OSC1, SP_OSCFREQ, "Osc1Pitch", "osc1pitch", 0.0, 1.0, 0.0, processOsc1Pitch)
	PARAM(OSC2P, PG_OSC2, SP_OSCFREQ, "Osc2Pitch", "osc2pitch", 0.0, 1.0, 0.0, processOsc2Pitch)
	PARAM_NULL(OSCQuantize_NOTUSED, "PitchQuant_NOTUSED", "pitchquant_noused")
	PARAM_NULL(OSC1Saw_NOTUSED, "Osc1Saw_NOTUSED", "osc1saw_notused")
	PARAM_NULL(OSC1Pul_NOTUSED, "Osc1Pulse_NOTUSED", "osc1pulse_notused")
	PARAM_NULL(OSC2Saw_NOTUSED, "Osc2Saw_NOTUSED", "osc1saw_notused")
	PARAM_NULL(OSC2Pul_NOTUSED, "Osc2Pulse_NOTUSED", "osc2pulse_notused")
	PARAM(OSC1PW, PG_OSC1, SP_NONE, "Osc1PW", "osc1pw", 0.0, 1.0, 0.0, processOsc1PulseWidth)
	PARAM(HPFFREQ, PG_VCA, SP_NONE, "HPFfreq", "hpffreq", 0.0, 1.0, 0.0, processHPFfreq)
	PARAM_NULL(ENVPITCH_NOTUSED, "EnvelopeToPitch_NOTUSED", "envelopetopitch_notused")
	PARAM(OSC1MIX, PG_MIXER, SP_NONE, "Osc1Mix", "osc1mix", 0.0, 1.0, 0.0, processOsc1Mix)
	PARAM(OSC2MIX, PG_MIXER, SP_NONE, "Osc2Mix", "osc2mix", 0.0, 1.0, 1.0, processOsc2Mix)
	PARAM(OSC3MIX, PG_MIXER, SP_NONE, "Osc3Mix", "osc3mix", 0.0, 1.0, 0.0, processOsc3Mix)
	PARAM(FLT_KF, PG_FILTER, SP_NONE, "FilterKeyFollow", "filterkeyfollow", 0.0, 1.0, 0.0, processFilterKeyFollow)
	PARAM(CUTOFF, PG_FILTER, SP_NONE, "Cutoff", "cutoff", 0.0, 1.0, 1.0, processCutoff)
	PARAM(RESONANCE, PG_FILTER, SP_NONE, "Resonance", "resonance", 0.0, 1.0, 0.0, processResonance)
	PARAM(RESPONSE, PG_FILTERCFG, SP_NONE, "Response", "response", 0.0, 1.0, 0.0, processResponse)
	PARAM(OVERSAMPLE, PG_DSP, SP_ONOFF, "Oversample", "oversample", 0.0, 1.0, 1.0, processOversampling)
	PARAM_NULL(BANDPASS_NOTUSED, "BandpassBlend_NOTUSED", "bandpassblend_notused")
	PARAM_NULL(FOURPOLE_NOTUSED, "FourPole_NOTUSED", "fourpole_notused")
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
	PARAM_NULL(PAN1_NOTUSED, "Pan1_NOTUSED", "pan1_notused")
	PARAM_NULL(PAN2_NOTUSED, "Pan2_NOTUSED", "pan2_notused")
	PARAM_NULL(PAN3_NOTUSED, "Pan3_NOTUSED", "pan3_notused")
	PARAM_NULL(PAN4_NOTUSED, "Pan4_NOTUSED", "pan4_notused")
	PARAM_NULL(PAN5_NOTUSED, "Pan5_NOTUSED", "pan5_notused")
	PARAM_NULL(PAN6_NOTUSED, "Pan6_NOTUSED", "pan6_notused")
	PARAM_NULL(PAN7_NOTUSED, "Pan7_NOTUSED", "pan7_notused")
	PARAM_NULL(PAN8_NOTUSED, "Pan8_NOTUSED", "pan8_notused")
	PARAM(UNUSED_2, PG_MISC, SP_NONE, "Unused 2", "unused_2", 0.0, 1.0, 0.0, procUnused2)
	PARAM(ECONOMY_MODE, PG_DSP, SP_ONOFF, "EconomyMode", "economymode", 0.0, 1.0, 1.0, procEconomyMode)
	PARAM(LFO1SYNC, PG_LFO1, SP_LFOSYNC, "Lfo1Sync", "lfo1sync", 0.0, 1.0, 0.0, procLfo1Sync)
	PARAM_NULL(PW_ENV_NOTUSED, "PwEnv_NOTUSED", "pwenv_notused")
	PARAM_NULL(PW_ENV_BOTH_NOTUSED, "PwEnvBoth_NOTUSED", "pwenvboth_notused")
	PARAM_NULL(ENV_PITCH_BOTH_NOTUSED, "EnvPitchBoth_NOTUSED", "envpitchboth_notused")
	PARAM(FENV_INVERT, PG_FILTERCFG, SP_ONOFF, "FenvInvert", "fenvinvert", 0.0, 1.0, 0.0, processInvertFenv)
	PARAM(OSC2PW, PG_OSC2, SP_NONE, "Osc2PW", "osc2pw", 0.0, 1.0, 0.0, processOsc2PulseWidth)
	PARAM(LEVEL_DIF, PG_SPREAD, SP_NONE, "LevelSpread", "levelspread", 0.0, 1.0, 0.3, processLoudnessSpread)
	PARAM_NULL(SELF_OSC_PUSH_NOTUSED, "SelfOscPush_NOTUSED", "selfoscpush_notused")
	PARAM(OSC1WAVE, PG_OSC1, SP_OSCWAVE, "Osc1Wave", "osc1wave", 0.0, 1.0, 0.25, processOsc1Wave)
	PARAM(OSC2WAVE, PG_OSC2, SP_OSCWAVE, "Osc2Wave", "osc2wave", 0.0, 1.0, 0.25, processOsc2Wave)
	PARAM(FREL, PG_REL, SP_NONE, "FilterRelease", "filterrelease", 0.0, 1.0, 0.0, processFilterEnvelopeRelease)
	PARAM(LREL, PG_REL, SP_NONE, "Release", "release", 0.0, 1.0, 0.0, processLoudnessEnvelopeRelease)
	PARAM_NULL(FILTER_TYPE, "FilterType_NOTUSED", "filtertype_unused")
	PARAM(OSC1FLTMOD, PG_OSC_COM, SP_NONE, "Osc1FilterMod", "osc1filtermod", 0.0, 1.0, 0.0, processOsc1FltMod)

        // ReSet to Zero (lowest) voice (default cyclic)
	PARAM(ASGN_RSZ, PG_KEYASGN, SP_ONOFF, "KeyAssignRsz", "keyassignrsz", 0.0, 1.0, 0.0, procKeyAsgnRsz)
        // Prefer assign to voice previously with same note
	PARAM(ASGN_MEM, PG_KEYASGN, SP_ONOFF, "KeyAssignMem", "keyassignmem", 0.0, 1.0, 0.0, procKeyAsgnMem)
        // Rob a playing voice if no unplaying available
        // two modes: oldest (O) and next-to-lowest (NL)
	PARAM(ASGN_ROB, PG_KEYASGN, SP_ROB, "KeyAssignRob", "keyassignrob", 0.0, 1.0, 0.0, procKeyAsgnRob)
        // Restore mode: Store notes until voice available
	PARAM(ASGN_RES, PG_KEYASGN, SP_ONOFF, "KeyAssignRes", "keyassignres", 0.0, 1.0, 0.0, procKeyAsgnRes)
        // Single trig: behavior during rob and restore
	PARAM(ASGN_STRG, PG_KEYASGN, SP_ONOFF, "KeyAssignStrg", "keyassignstrg", 0.0, 1.0, 0.0, procKeyAsgnStrg)

	PARAM(LFO2SYNC, PG_LFO2, SP_LFOSYNC, "Lfo2Sync", "lfo2sync", 0.0, 1.0, 0.0, procLfo2Sync)

	PARAM(LFOSPREAD, PG_SPREAD, SP_NONE, "LfoSpread", "lfospread", 0.0, 1.0, 0.0, processLfoSpread)

	PARAM(PANSPREAD, PG_SPREAD, SP_NONE, "PanSpread", "panspread", 0.0, 1.0, 0.1, processPanSpread)

	PARAM_NULL(GATK_NOTUSED, "GenAttack_NOTUSED", "genattack_notused")
	PARAM_NULL(GDEC_NOTUSED, "GenDecay_NOTUSED", "gendecay_notused")
	PARAM_NULL(GSUS_NOTUSED, "GenSustain_NOTUSED", "gensustain_notused")
	PARAM_NULL(GSUST_NOTUSED, "GenSustainTime_NOTUSED", "gensustaintime_notused")
	PARAM_NULL(GREL_NOTUSED, "GenRelease_NOTUSED", "genrelease_notused")

	PARAM_NULL(GAMT_NOTUSED, "GenEnvAmount_NOTUSED", "genenvamount_notused")
	PARAM_NULL(GDEST_NOTUSED, "GenEnvDest_NOTUSED", "genenvdest_notused")
	PARAM_NULL(GUNI_NOTUSED, "GenEnvUnipol_NOTUSED", "genenvunipol_notused")
	PARAM_NULL(GINV_NOTUSED, "GenEnvInvert_NOTUSED", "genenvinvert_notused")

	PARAM_NULL(VGENENV_NOTUSED, "VGenFactor_NOTUSED", "vgenfactor_notusd")
	PARAM(VCADRIVE, PG_VCA, SP_NONE, "VCADrive", "vcadrive", 0.0, 1.0, 0.0, processVCADrive)

	PARAM(OSCSYNC_LEVEL, PG_OSC_COM, SP_NONE, "SyncLevel", "synclevel", 0.0, 1.0, 0.0, processOsc2SyncLevel)

	PARAM(OSC3WAVE, PG_MIXER, SP_OSC3WAVE, "Osc3Wave", "osc3wave",0.0, 1.0, 0.0, processOsc3Wave)

	// 111
	PARAM(VEL_SCALE, PG_CONTR, SP_NONE, "VelocityScale", "velocityscale", 0.0, 1.0, 0.0, procVelocityScale)
	PARAM(AT_SCALE, PG_CONTR, SP_NONE, "AfterTouchScale", "aftertouchscale", 0.0, 1.0, 0.0, procAfterTouchScale)

	// 113
	PARAM(LFO1_AMT_CTRL, PG_LFO1, SP_LFOCONTR, "Lfo1AmtCont", "lfo1amtcont", 0.0, 1.0, 0.0, procLfo1Controller)
	PARAM(LFO2_AMT_CTRL, PG_LFO2, SP_LFOCONTR, "Lfo2AmtCont", "lfo2amtcont", 0.0, 1.0, 0.0, procLfo2Controller)

	// 115
	PARAM(LFO1_CONTRAMT, PG_LFO1, SP_NONE, "Lfo1ContrAmt", "lfo1contramt", 0.0, 1.0, 0.0, procLfo1ControllerAmt)
	PARAM(LFO2_CONTRAMT, PG_LFO2, SP_NONE, "Lfo2ContrAmt", "lfo2contramt", 0.0, 1.0, 0.0, procLfo2ControllerAmt)

	// 117
	PARAM(LFO1_POLARITY, PG_LFO1, SP_POLARITY, "Lfo1Polarity", "lfo1polarity", 0.0, 1.0, 0.0, procLfo1Polarity)
	PARAM(LFO2_POLARITY, PG_LFO2, SP_POLARITY, "Lfo2Polarity", "lfo2polarity", 0.0, 1.0, 0.0, procLfo2Polarity)

	// 119
	PARAM(OSC_KEY_SYNC, PG_OSC_COM, SP_ONOFF, "OscKeySync", "osckeysync", 0.0, 1.0, 0.0, procOscKeySync)
	PARAM(ENV_RST, PG_KEYASGN, SP_ONOFF, "EnvRst", "envrst", 0.0, 1.0, 0.0, procEnvRst)

	// 121
	PARAM(FENV_LINEAR, PG_FILTERCFG, SP_ONOFF, "FenvLinear", "fenvlinear", 0.0, 1.0, 0.0, procFenvLinear)
	PARAM(ENV_MODE, PG_VCA, SP_ENVMODE, "EnvMode", "envmode", 0.0, 1.0, 0.0, procEnvMode)
