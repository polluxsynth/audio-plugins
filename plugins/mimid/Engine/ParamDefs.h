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
	// ParamsEnum.h just extracts the parameter symbol in order to create
	// the Parameters enum (order is important).
	// PluginProcessor sets up the process callbacks and names of the
	// parameters (and in this case the order is not important).

	PARAM(UNUSED_1, "Unused 1", 0.0, 1.0, 0.0, procUnused1)
	PARAM(VOLUME, "Volume", 0.0, 1.0, 0.5, processVolume)
	PARAM(VOICE_COUNT, "VoiceCount", 0.0, 1.0, 1.0, setVoiceCount)
	PARAM(TUNE, "Tune", 0.0, 1.0, 0.5, processTune)
	PARAM(OCTAVE, "Octave", 0.0, 1.0, 0.5, processOctave)
	PARAM(BENDRANGE, "BendRange", 0.0, 1.0, 0.0, procPitchWheelAmount)
	PARAM(BENDDEST, "BendDest", 0.0, 1.0, 0.0, procPitchWheelDest)
	PARAM_NULL(LEGATOMODE_NOTUSED, "LegatoMode_NOTUSED")
	PARAM(LFO2FREQ, "Lfo2Frequency", 0.0, 1.0, 0.6, processLfo2Frequency)
	PARAM(VFLTENV, "VFltFactor", 0.0, 1.0, 0.0, procFltVelocityAmount)
	PARAM(VAMPENV, "VAmpFactor", 0.0, 1.0, 0.0, procAmpVelocityAmount)

	PARAM_NULL(ASPLAYEDALLOCATION_NOTUSED, "AsPlayedAllocation_NOTUSED")
	PARAM(PORTAMENTO, "Portamento", 0.0, 1.0, 0.0, processPortamento)
	PARAM(UNISON, "Unison", 0.0, 1.0, 0.0, processUnison) // TODO part of keyasgn?
	PARAM(UDET, "OscSpread", 0.0, 1.0, 0.2, processOscSpread)
	PARAM(OSC1_DET, "Oscillator1detune", 0.0, 1.0, 0.0, processOsc1Det)
	PARAM(LFO1FREQ, "Lfo1Frequency", 0.0, 1.0, 0.0, processLfo1Frequency)
	PARAM(LFO1WAVE, "Lfo1Wave", 0.0, 1.0, 0.0, processLfo1Wave)
	PARAM(LFO2WAVE, "Lfo2Wave", 0.0, 1.0, 0.0, processLfo2Wave)
	PARAM_NULL(LFOSHWAVE_UNUSED, "LfoSampleHoldWave")
	PARAM(LFO1AMT, "Lfo1Amount", 0.0, 1.0, 0.0, processLfo1Amt)
	PARAM(LFO2AMT, "Lfo2Amount", 0.0, 1.0, 0.0, processLfo2Amt)
	PARAM(LFO1DEST, "Lfo1Dest", 0.0, 1.0, 0.0, processLfo1Dest)
	PARAM(LFO2DEST, "Lfo2Dest", 0.0, 1.0, 0.0, processLfo2Dest)
	PARAM_NULL(LFOFILTER_UNUSED, "LfoFilter")
	PARAM_NULL(LFOPW1_UNUSED, "LfoPw1_NOTUSED")
	PARAM_NULL(LFOPW2_UNUSED, "LfoPw2_NOTUSED")
	PARAM(OSC2_DET, "Oscillator2detune", 0.0, 1.0, 0.0, processOsc2Det)
	PARAM(XMOD, "Xmod", 0.0, 1.0, 0.0, processOsc2Xmod)
	PARAM(OSC1P, "Osc1Pitch", 0.0, 1.0, 0.0, processOsc1Pitch)
	PARAM(OSC2P, "Osc2Pitch", 0.0, 1.0, 0.0, processOsc2Pitch)
	PARAM_NULL(OSCQuantize_NOTUSED, "PitchQuant_NOTUSED")
	PARAM_NULL(OSC1Saw_NOTUSED, "Osc1Saw_NOTUSED")
	PARAM_NULL(OSC1Pul_NOTUSED, "Osc1Pulse_NOTUSED")
	PARAM_NULL(OSC2Saw_NOTUSED, "Osc2Saw_NOTUSED")
	PARAM_NULL(OSC2Pul_NOTUSED, "Osc2Pulse_NOTUSED")
	PARAM(OSC1PW, "Osc1PW", 0.0, 1.0, 0.0, processOsc1PulseWidth)
	PARAM(HPFFREQ, "HPFfreq", 0.0, 1.0, 0.0, processHPFfreq)
	PARAM_NULL(ENVPITCH_NOTUSED, "EnvelopeToPitch_NOTUSED")
	PARAM(OSC1MIX, "Osc1Mix", 0.0, 1.0, 0.0, processOsc1Mix)
	PARAM(OSC2MIX, "Osc2Mix", 0.0, 1.0, 1.0, processOsc2Mix)
	PARAM(OSC3MIX, "Osc3Mix", 0.0, 1.0, 0.0, processOsc3Mix)
	PARAM(FLT_KF, "FilterKeyFollow", 0.0, 1.0, 0.0, processFilterKeyFollow)
	PARAM(CUTOFF, "Cutoff", 0.0, 1.0, 1.0, processCutoff)
	PARAM(RESONANCE, "Resonance", 0.0, 1.0, 0.0, processResonance)
	PARAM(RESPONSE, "Response", 0.0, 1.0, 0.0, processResponse)
	PARAM(OVERSAMPLE, "Oversample", 0.0, 1.0, 1.0, processOversampling)
	PARAM_NULL(BANDPASS_NOTUSED, "BandpassBlend_NOTUSED")
	PARAM_NULL(FOURPOLE_NOTUSED, "FourPole_NOTUSED")
	PARAM(ENVELOPE_AMT, "FilterEnvAmount", 0.0, 1.0, 0.0, processFilterEnvelopeAmt)
	PARAM(LATK, "Attack", 0.0, 1.0, 0.0, processLoudnessEnvelopeAttack)
	PARAM(LDEC, "Decay", 0.0, 1.0, 0.0, processLoudnessEnvelopeDecay)
	PARAM(LSUS, "Sustain", 0.0, 1.0, 1.0, processLoudnessEnvelopeSustain)
	PARAM(LSUST, "SustainTime", 0.0, 1.0, 1.0, processLoudnessEnvelopeSustainTime)
	PARAM(FATK, "FilterAttack", 0.0, 1.0, 0.0, processFilterEnvelopeAttack)
	PARAM(FDEC, "FilterDecay", 0.0, 1.0, 0.0, processFilterEnvelopeDecay)
	PARAM(FSUS, "FilterSustain", 0.0, 1.0, 1.0, processFilterEnvelopeSustain)
	PARAM(FSUST, "FilterSustainTime", 0.0, 1.0, 1.0, processFilterEnvelopeSustainTime)
	PARAM(ENVDER, "EnvelopeSpread", 0.0, 1.0, 0.0, processEnvelopeSpread)
	PARAM(FILTERDER, "FilterSpread", 0.0, 1.0, 0.3, processFilterSpread)
	PARAM(PORTADER, "PortamentoSpread", 0.0, 1.0, 0.3, processPortamentoSpread)
	PARAM_NULL(PAN1_NOTUSED, "Pan1_NOTUSED")
	PARAM_NULL(PAN2_NOTUSED, "Pan2_NOTUSED")
	PARAM_NULL(PAN3_NOTUSED, "Pan3_NOTUSED")
	PARAM_NULL(PAN4_NOTUSED, "Pan4_NOTUSED")
	PARAM_NULL(PAN5_NOTUSED, "Pan5_NOTUSED")
	PARAM_NULL(PAN6_NOTUSED, "Pan6_NOTUSED")
	PARAM_NULL(PAN7_NOTUSED, "Pan7_NOTUSED")
	PARAM_NULL(PAN8_NOTUSED, "Pan8_NOTUSED")
	PARAM(UNUSED_2, "Unused 2", 0.0, 1.0, 0.0, procUnused2)
	PARAM(ECONOMY_MODE, "EconomyMode", 0.0, 1.0, 1.0, procEconomyMode)
	PARAM(LFO1SYNC, "Lfo1Sync", 0.0, 1.0, 0.0, procLfo1Sync)
	PARAM_NULL(PW_ENV_NOTUSED, "PwEnv_NOTUSED")
	PARAM_NULL(PW_ENV_BOTH_NOTUSED, "PwEnvBoth_NOTUSED")
	PARAM_NULL(ENV_PITCH_BOTH_NOTUSED, "EnvPitchBoth_NOTUSED")
	PARAM(FENV_INVERT, "FenvInvert", 0.0, 1.0, 0.0, processInvertFenv)
	PARAM(OSC2PW, "Osc2PW", 0.0, 1.0, 0.0, processOsc2PulseWidth)
	PARAM(LEVEL_DIF, "LevelSpread", 0.0, 1.0, 0.3, processLoudnessSpread)
	PARAM_NULL(SELF_OSC_PUSH_NOTUSED, "SelfOscPush_NOTUSED")
	PARAM(OSC1WAVE, "Osc1Wave", 0.0, 1.0, 0.25, processOsc1Wave)
	PARAM(OSC2WAVE, "Osc2Wave", 0.0, 1.0, 0.25, processOsc2Wave)
	PARAM(FREL, "FilterRelease", 0.0, 1.0, 0.0, processFilterEnvelopeRelease)
	PARAM(LREL, "Release", 0.0, 1.0, 0.0, processLoudnessEnvelopeRelease)
	PARAM_NULL(FILTER_TYPE, "FilterType_NOTUSED")
	PARAM(OSC1FLTMOD, "Osc1FilterMod", 0.0, 1.0, 0.0, processOsc1FltMod)

        // ReSet to Zero (lowest) voice (default cyclic)
	PARAM(ASGN_RSZ, "KeyAssignRsz", 0.0, 1.0, 0.0, procKeyAsgnRsz)
        // Prefer assign to voice previously with same note
	PARAM(ASGN_MEM, "KeyAssignMem", 0.0, 1.0, 0.0, procKeyAsgnMem)
        // Rob a playing voice if no unplaying available
        // two modes: oldest (O) and next-to-lowest (NL)
	PARAM(ASGN_ROB, "KeyAssignRob", 0.0, 1.0, 0.0, procKeyAsgnRob)
        // Restore mode: Store notes until voice available
	PARAM(ASGN_RES, "KeyAssignRes", 0.0, 1.0, 0.0, procKeyAsgnRes)
        // Single trig: behavior during rob and restore
	PARAM(ASGN_STRG, "KeyAssignStrg", 0.0, 1.0, 0.0, procKeyAsgnStrg)

	PARAM(LFO2SYNC, "Lfo2Sync", 0.0, 1.0, 0.0, procLfo2Sync)

	PARAM(LFOSPREAD, "LfoSpread", 0.0, 1.0, 0.0, processLfoSpread)

	PARAM(PANSPREAD, "PanSpread", 0.0, 1.0, 0.1, processPanSpread)

	PARAM_NULL(GATK_NOTUSED, "GenAttack_NOTUSED")
	PARAM_NULL(GDEC_NOTUSED, "GenDecay_NOTUSED")
	PARAM_NULL(GSUS_NOTUSED, "GenSustain_NOTUSED")
	PARAM_NULL(GSUST_NOTUSED, "GenSustainTime_NOTUSED")
	PARAM_NULL(GREL_NOTUSED, "GenRelease_NOTUSED")

	PARAM_NULL(GAMT_NOTUSED, "GenEnvAmount_NOTUSED")
	PARAM_NULL(GDEST_NOTUSED, "GenEnvDest_NOTUSED")
	PARAM_NULL(GUNI_NOTUSED, "GenEnvUnipol_NOTUSED")
	PARAM_NULL(GINV_NOTUSED, "GenEnvInvert_NOTUSED")

	PARAM_NULL(VGENENV_NOTUSED, "VGenFactor_NOTUSED")
	PARAM(VCADRIVE, "VCADrive", 0.0, 1.0, 0.0, processVCADrive)

	PARAM(OSCSYNC_LEVEL, "SyncLevel", 0.0, 1.0, 0.0, processOsc2SyncLevel)

	PARAM(OSC3WAVE, "Osc3Wave", 0.0, 1.0, 0.0, processOsc3Wave)

	// 111
	PARAM(VEL_SCALE, "VelocityScale", 0.0, 1.0, 0.0, procVelocityScale)
	PARAM(AT_SCALE, "AfterTouchScale", 0.0, 1.0, 0.0, procAfterTouchScale)

	// 113
	PARAM(LFO1_AMT_CTRL, "Lfo1AmtCont", 0.0, 1.0, 0.0, procLfo1Controller)
	PARAM(LFO2_AMT_CTRL, "Lfo2AmtCont", 0.0, 1.0, 0.0, procLfo2Controller)

	// 115
	PARAM(LFO1_CONTRAMT, "Lfo1ContrAmt", 0.0, 1.0, 0.0, procLfo1ControllerAmt)
	PARAM(LFO2_CONTRAMT, "Lfo2ContrAmt", 0.0, 1.0, 0.0, procLfo2ControllerAmt)

	// 117
	PARAM(LFO1_POLARITY, "Lfo1Polarity", 0.0, 1.0, 0.0, procLfo1Polarity)
	PARAM(LFO2_POLARITY, "Lfo2Polarity", 0.0, 1.0, 0.0, procLfo2Polarity)

	// 119
	PARAM(OSC_KEY_SYNC, "OscKeySync", 0.0, 1.0, 0.0, procOscKeySync)
	PARAM(ENV_RST, "EnvRst", 0.0, 1.0, 0.0, procEnvRst)

	// 121
	PARAM(FENV_LINEAR, "FenvLinear", 0.0, 1.0, 0.0, procFenvLinear)
	PARAM(ENV_MODE, "EnvMode", 0.0, 1.0, 0.0, procEnvMode)
