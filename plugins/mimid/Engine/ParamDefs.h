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
	PARAMGROUP(PG_UNUSED, "Unused", "g000_unused")
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

	PARAM(UNUSED_1, "Unused 1", "unused_1", 0.0, 1.0, 0.0, procUnused1)
	PARAM(VOLUME, "Volume", "volume", 0.0, 1.0, 0.5, processVolume)
	PARAM(VOICE_COUNT, "VoiceCount", "voicecount", 0.0, 1.0, 1.0, setVoiceCount)
	PARAM(TUNE, "Tune", "tune", 0.0, 1.0, 0.5, processTune)
	PARAM(OCTAVE, "Octave", "octave", 0.0, 1.0, 0.5, processOctave)
	PARAM(BENDRANGE, "BendRange", "bendrange", 0.0, 1.0, 0.0, procPitchWheelAmount)
	PARAM(BENDDEST, "BendDest", "benddest", 0.0, 1.0, 0.0, procPitchWheelDest)
	PARAM_NULL(LEGATOMODE_NOTUSED, "LegatoMode_NOTUSED", "legatomode_unused")
	PARAM(LFO2FREQ, "Lfo2Frequency", "lfo2frequency", 0.0, 1.0, 0.6, processLfo2Frequency)
	PARAM(VFLTENV, "VFltFactor", "vfltfactor", 0.0, 1.0, 0.0, procFltVelocityAmount)
	PARAM(VAMPENV, "VAmpFactor", "vampfactor", 0.0, 1.0, 0.0, procAmpVelocityAmount)

	PARAM_NULL(ASPLAYEDALLOCATION_NOTUSED, "AsPlayedAllocation_NOTUSED", "asplayedallocation_notused")
	PARAM(PORTAMENTO, "Portamento", "portamento", 0.0, 1.0, 0.0, processPortamento)
	PARAM(UNISON, "Unison", "unison", 0.0, 1.0, 0.0, processUnison) // TODO part of keyasgn?
	PARAM(UDET, "OscSpread", "oscspread", 0.0, 1.0, 0.2, processOscSpread)
	PARAM(OSC1_DET, "Oscillator1detune", "oscillator1detune", 0.0, 1.0, 0.0, processOsc1Det)
	PARAM(LFO1FREQ, "Lfo1Frequency", "lfo1frequency", 0.0, 1.0, 0.0, processLfo1Frequency)
	PARAM(LFO1WAVE, "Lfo1Wave", "lfo1wave", 0.0, 1.0, 0.0, processLfo1Wave)
	PARAM(LFO2WAVE, "Lfo2Wave", "lfo2wave", 0.0, 1.0, 0.0, processLfo2Wave)
	PARAM_NULL(LFOSHWAVE_UNUSED, "LfoSampleHoldWave_NOTUSED", "lfosampleholdwave_notused")
	PARAM(LFO1AMT, "Lfo1Amount", "lfo1amount", 0.0, 1.0, 0.0, processLfo1Amt)
	PARAM(LFO2AMT, "Lfo2Amount", "lfo2amount", 0.0, 1.0, 0.0, processLfo2Amt)
	PARAM(LFO1DEST, "Lfo1Dest", "lfo1dest", 0.0, 1.0, 0.0, processLfo1Dest)
	PARAM(LFO2DEST, "Lfo2Dest", "lfo2dest", 0.0, 1.0, 0.0, processLfo2Dest)
	PARAM_NULL(LFOFILTER_UNUSED, "LfoFilter_NOTUSED", "lfofilter_notused")
	PARAM_NULL(LFOPW1_UNUSED, "LfoPw1_NOTUSED", "lfopw1_notused")
	PARAM_NULL(LFOPW2_UNUSED, "LfoPw2_NOTUSED", "lfopw2_notused")
	PARAM(OSC2_DET, "Oscillator2detune", "oscillator2detune", 0.0, 1.0, 0.0, processOsc2Det)
	PARAM(XMOD, "Xmod", "xmod", 0.0, 1.0, 0.0, processOsc2Xmod)
	PARAM(OSC1P, "Osc1Pitch", "osc1pitch", 0.0, 1.0, 0.0, processOsc1Pitch)
	PARAM(OSC2P, "Osc2Pitch", "osc2pitch", 0.0, 1.0, 0.0, processOsc2Pitch)
	PARAM_NULL(OSCQuantize_NOTUSED, "PitchQuant_NOTUSED", "pitchquant_noused")
	PARAM_NULL(OSC1Saw_NOTUSED, "Osc1Saw_NOTUSED", "osc1saw_notused")
	PARAM_NULL(OSC1Pul_NOTUSED, "Osc1Pulse_NOTUSED", "osc1pulse_notused")
	PARAM_NULL(OSC2Saw_NOTUSED, "Osc2Saw_NOTUSED", "osc1saw_notused")
	PARAM_NULL(OSC2Pul_NOTUSED, "Osc2Pulse_NOTUSED", "osc2pulse_notused")
	PARAM(OSC1PW, "Osc1PW", "osc1pw", 0.0, 1.0, 0.0, processOsc1PulseWidth)
	PARAM(HPFFREQ, "HPFfreq", "hpffreq", 0.0, 1.0, 0.0, processHPFfreq)
	PARAM_NULL(ENVPITCH_NOTUSED, "EnvelopeToPitch_NOTUSED", "envelopetopitch_notused")
	PARAM(OSC1MIX, "Osc1Mix", "osc1mix", 0.0, 1.0, 0.0, processOsc1Mix)
	PARAM(OSC2MIX, "Osc2Mix", "osc2mix", 0.0, 1.0, 1.0, processOsc2Mix)
	PARAM(OSC3MIX, "Osc3Mix", "osc3mix", 0.0, 1.0, 0.0, processOsc3Mix)
	PARAM(FLT_KF, "FilterKeyFollow", "filterkeyfollow", 0.0, 1.0, 0.0, processFilterKeyFollow)
	PARAM(CUTOFF, "Cutoff", "cutoff", 0.0, 1.0, 1.0, processCutoff)
	PARAM(RESONANCE, "Resonance", "resonance", 0.0, 1.0, 0.0, processResonance)
	PARAM(RESPONSE, "Response", "response", 0.0, 1.0, 0.0, processResponse)
	PARAM(OVERSAMPLE, "Oversample", "oversample", 0.0, 1.0, 1.0, processOversampling)
	PARAM_NULL(BANDPASS_NOTUSED, "BandpassBlend_NOTUSED", "bandpassblend_notused")
	PARAM_NULL(FOURPOLE_NOTUSED, "FourPole_NOTUSED", "fourpole_notused")
	PARAM(ENVELOPE_AMT, "FilterEnvAmount", "filterenvamount", 0.0, 1.0, 0.0, processFilterEnvelopeAmt)
	PARAM(LATK, "Attack", "attack", 0.0, 1.0, 0.0, processLoudnessEnvelopeAttack)
	PARAM(LDEC, "Decay", "decay", 0.0, 1.0, 0.0, processLoudnessEnvelopeDecay)
	PARAM(LSUS, "Sustain", "sustain", 0.0, 1.0, 1.0, processLoudnessEnvelopeSustain)
	PARAM(LSUST, "SustainTime", "sustaintime", 0.0, 1.0, 1.0, processLoudnessEnvelopeSustainTime)
	PARAM(FATK, "FilterAttack", "filterattack", 0.0, 1.0, 0.0, processFilterEnvelopeAttack)
	PARAM(FDEC, "FilterDecay", "filterdecay", 0.0, 1.0, 0.0, processFilterEnvelopeDecay)
	PARAM(FSUS, "FilterSustain", "filtersustain", 0.0, 1.0, 1.0, processFilterEnvelopeSustain)
	PARAM(FSUST, "FilterSustainTime", "filtersustaintime", 0.0, 1.0, 1.0, processFilterEnvelopeSustainTime)
	PARAM(ENVDER, "EnvelopeSpread", "envelopspread", 0.0, 1.0, 0.0, processEnvelopeSpread)
	PARAM(FILTERDER, "FilterSpread", "filtespread", 0.0, 1.0, 0.3, processFilterSpread)
	PARAM(PORTADER, "PortamentoSpread", "portamentospread", 0.0, 1.0, 0.3, processPortamentoSpread)
	PARAM_NULL(PAN1_NOTUSED, "Pan1_NOTUSED", "pan1_notused")
	PARAM_NULL(PAN2_NOTUSED, "Pan2_NOTUSED", "pan2_notused")
	PARAM_NULL(PAN3_NOTUSED, "Pan3_NOTUSED", "pan3_notused")
	PARAM_NULL(PAN4_NOTUSED, "Pan4_NOTUSED", "pan4_notused")
	PARAM_NULL(PAN5_NOTUSED, "Pan5_NOTUSED", "pan5_notused")
	PARAM_NULL(PAN6_NOTUSED, "Pan6_NOTUSED", "pan6_notused")
	PARAM_NULL(PAN7_NOTUSED, "Pan7_NOTUSED", "pan7_notused")
	PARAM_NULL(PAN8_NOTUSED, "Pan8_NOTUSED", "pan8_notused")
	PARAM(UNUSED_2, "Unused 2", "unused_2", 0.0, 1.0, 0.0, procUnused2)
	PARAM(ECONOMY_MODE, "EconomyMode", "economymode", 0.0, 1.0, 1.0, procEconomyMode)
	PARAM(LFO1SYNC, "Lfo1Sync", "lfo1sync", 0.0, 1.0, 0.0, procLfo1Sync)
	PARAM_NULL(PW_ENV_NOTUSED, "PwEnv_NOTUSED", "pwenv_notused")
	PARAM_NULL(PW_ENV_BOTH_NOTUSED, "PwEnvBoth_NOTUSED", "pwenvboth_notused")
	PARAM_NULL(ENV_PITCH_BOTH_NOTUSED, "EnvPitchBoth_NOTUSED", "envpitchboth_notused")
	PARAM(FENV_INVERT, "FenvInvert", "fenvinvert", 0.0, 1.0, 0.0, processInvertFenv)
	PARAM(OSC2PW, "Osc2PW", "osc2pw", 0.0, 1.0, 0.0, processOsc2PulseWidth)
	PARAM(LEVEL_DIF, "LevelSpread", "levelspread", 0.0, 1.0, 0.3, processLoudnessSpread)
	PARAM_NULL(SELF_OSC_PUSH_NOTUSED, "SelfOscPush_NOTUSED", "selfoscpush_notused")
	PARAM(OSC1WAVE, "Osc1Wave", "osc1wave", 0.0, 1.0, 0.25, processOsc1Wave)
	PARAM(OSC2WAVE, "Osc2Wave", "osc2wave", 0.0, 1.0, 0.25, processOsc2Wave)
	PARAM(FREL, "FilterRelease", "filterrelease", 0.0, 1.0, 0.0, processFilterEnvelopeRelease)
	PARAM(LREL, "Release", "release", 0.0, 1.0, 0.0, processLoudnessEnvelopeRelease)
	PARAM_NULL(FILTER_TYPE, "FilterType_NOTUSED", "filtertype_unused")
	PARAM(OSC1FLTMOD, "Osc1FilterMod", "osc1filtermod", 0.0, 1.0, 0.0, processOsc1FltMod)

        // ReSet to Zero (lowest) voice (default cyclic)
	PARAM(ASGN_RSZ, "KeyAssignRsz", "keyassignrsz", 0.0, 1.0, 0.0, procKeyAsgnRsz)
        // Prefer assign to voice previously with same note
	PARAM(ASGN_MEM, "KeyAssignMem", "keyassignmem", 0.0, 1.0, 0.0, procKeyAsgnMem)
        // Rob a playing voice if no unplaying available
        // two modes: oldest (O) and next-to-lowest (NL)
	PARAM(ASGN_ROB, "KeyAssignRob", "keyassignrob", 0.0, 1.0, 0.0, procKeyAsgnRob)
        // Restore mode: Store notes until voice available
	PARAM(ASGN_RES, "KeyAssignRes", "keyassignres", 0.0, 1.0, 0.0, procKeyAsgnRes)
        // Single trig: behavior during rob and restore
	PARAM(ASGN_STRG, "KeyAssignStrg", "keyassignstrg", 0.0, 1.0, 0.0, procKeyAsgnStrg)

	PARAM(LFO2SYNC, "Lfo2Sync", "lfo2sync", 0.0, 1.0, 0.0, procLfo2Sync)

	PARAM(LFOSPREAD, "LfoSpread", "lfospread", 0.0, 1.0, 0.0, processLfoSpread)

	PARAM(PANSPREAD, "PanSpread", "panspread", 0.0, 1.0, 0.1, processPanSpread)

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
	PARAM(VCADRIVE, "VCADrive", "vcadrive", 0.0, 1.0, 0.0, processVCADrive)

	PARAM(OSCSYNC_LEVEL, "SyncLevel", "synclevel", 0.0, 1.0, 0.0, processOsc2SyncLevel)

	PARAM(OSC3WAVE, "Osc3Wave", "osc3wave",0.0, 1.0, 0.0, processOsc3Wave)

	// 111
	PARAM(VEL_SCALE, "VelocityScale", "velocityscale", 0.0, 1.0, 0.0, procVelocityScale)
	PARAM(AT_SCALE, "AfterTouchScale", "aftertouchscale", 0.0, 1.0, 0.0, procAfterTouchScale)

	// 113
	PARAM(LFO1_AMT_CTRL, "Lfo1AmtCont", "lfo1amtcont", 0.0, 1.0, 0.0, procLfo1Controller)
	PARAM(LFO2_AMT_CTRL, "Lfo2AmtCont", "lfo2amtcont", 0.0, 1.0, 0.0, procLfo2Controller)

	// 115
	PARAM(LFO1_CONTRAMT, "Lfo1ContrAmt", "lfo1contramt", 0.0, 1.0, 0.0, procLfo1ControllerAmt)
	PARAM(LFO2_CONTRAMT, "Lfo2ContrAmt", "lfo2contramt", 0.0, 1.0, 0.0, procLfo2ControllerAmt)

	// 117
	PARAM(LFO1_POLARITY, "Lfo1Polarity", "lfo1polarity", 0.0, 1.0, 0.0, procLfo1Polarity)
	PARAM(LFO2_POLARITY, "Lfo2Polarity", "lfo2polarity", 0.0, 1.0, 0.0, procLfo2Polarity)

	// 119
	PARAM(OSC_KEY_SYNC, "OscKeySync", "osckeysync", 0.0, 1.0, 0.0, procOscKeySync)
	PARAM(ENV_RST, "EnvRst", "envrst", 0.0, 1.0, 0.0, procEnvRst)

	// 121
	PARAM(FENV_LINEAR, "FenvLinear", "fenvlinear", 0.0, 1.0, 0.0, procFenvLinear)
	PARAM(ENV_MODE, "EnvMode", "envmode", 0.0, 1.0, 0.0, procEnvMode)
