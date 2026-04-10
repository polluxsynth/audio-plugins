/*
 * UILayoutDefs.h   -   UI layout definitions for MiMi-d
 *
 * Contains ONLY GUI-structural information: which page each module
 * is on, which parameter group it corresponds to, its direction and
 * grid position, and which parameters it contains.
 *
 * All parameter metadata (name, min, max, default, scale points) is
 * read directly from Engine/ParamDefs.h  -  nothing is duplicated here.
 *
 * Macros consumed (all have null defaults for safe re-inclusion):
 *
 *   UILAYOUT_PAGE(ID, LABEL)
 *       Declares a page tab. ID = 0, 1, ...
 *
 *   UILAYOUT_MODULE(PAGEID, PGID, DIR, GCOL, GROW)
 *   UILAYOUT_MODULE(PAGEID, PGID, DIR, GCOL, GROW, "Override title")
 *       Begins a module. PGID = PG_* enum from ParamDefs.h.
 *       DIR = WIDTH(n)  (VERT = WIDTH(1), HORIZ = WIDTH(16)).
 *       GCOL, GROW = grid column and row of first knob.
 *       Optional final string overrides the frame title derived from PGID.
 *
 *   UILAYOUT_MODULE_END
 *       Ends a module.
 *
 *   UILAYOUT_PARAM(PARAMNO)
 *       One knob. PARAMNO = Parameters enum value from ParamsEnum.h.
 *       All metadata looked up from ParamDefs.h tables at runtime.
 */

/*
 * ==============================================================================
 * This file is part of the MiMi-d synthesizer.
 *
 * Copyright 2026 Ricard Wanderlof
 *
 * This file may be licensed under the terms of of the
 * GNU General Public License Version 2 (the "GPL").
 *
 * Software distributed under the License is distributed
 * on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either
 * express or implied. See the GPL for the specific language
 * governing rights and limitations.
 *
 * You should have received a copy of the GPL along with this
 * program. If not, go to http://www.gnu.org/licenses/gpl.html
 * or write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 * ==============================================================================
 */

// Window size in knob grid units  -  update when layout changes.
// Columns = 0-based max column + 1, Rows = 0-based max row + 1.
#ifndef _UILAYOUT_SIZE_H_
#define _UILAYOUT_SIZE_H_

#define STR(S) STRINGIZE(S)
#define STRINGIZE(S) #S

#include "UIConstants.h"
UI_SIZE(14, 5)
// Plugin identity strings  -  single source of truth for name and description.
#define PLUGIN_NAME        "MiMi-d"
#define PLUGIN_DESCRIPTION "Polyphonic Virtual Analog Synthesizer"
#define PLUGIN_VERSION "Version " STR(PLUGIN_VERSION_MAJOR) "." \
                                  STR(PLUGIN_VERSION_MINOR) "." \
                                  STR(PLUGIN_VERSION_MICRO)
// Splash screen thanks text  -  nullptr-terminated array of lines.
static constexpr const char *SPLASH_TEXT[] = {
    "Thanks to:",
    "- 2DaT (Vadim Filatov) for the excellent OB-Xd plugin.",
    "- The Zynthian team for encouragement and support.",
    "  (in particular Fernando Moyano and Brian Walton).",
    "- Tomas Kubina for graphic block diagram in the manual.",
    "- Vadim Zavalishin for popularizing the topology-preserving transform filter design.",
    "- falkTX (Filipe Coelho) for the Distrho Plugin Framework.",
    nullptr
};
#define SPLASH_CLOSE_HINT_TEXT "Click or press Escape to close"

// Button strip label strings
static constexpr const char *BTN_LABEL_PAGE = "Toggle Page";
static constexpr const char *BTN_LABEL_UI   = "UI";
#endif /* _UILAYOUT_SIZE_H_ */

#ifndef UILAYOUT_PAGE
#define UILAYOUT_PAGE(ID, LABEL)
#endif
#ifndef UILAYOUT_MODULE
#define UILAYOUT_MODULE(PAGEID, PGID, DIR, GCOL, GROW, ...)
#endif
#ifndef UILAYOUT_MODULE_END
#define UILAYOUT_MODULE_END
#endif
#ifndef UILAYOUT_PARAM
#define UILAYOUT_PARAM(PARAMNO, ...)
#endif
#ifndef UILAYOUT_PARAM_POINTS
#define UILAYOUT_PARAM_POINTS(...)
#endif

// ===========================================================================
// PAGES
// ===========================================================================

UILAYOUT_PAGE(0, "Common")
UILAYOUT_PAGE(1, "Audio path")
UILAYOUT_PAGE(2, "Control")

// ===========================================================================
// PAGE 0  -  Common (always visible)
// ===========================================================================

UILAYOUT_MODULE(0, PG_MAIN, VERT, 0, 0)
    UILAYOUT_PARAM(VOLUME)
    UILAYOUT_PARAM(TUNE)
    UILAYOUT_PARAM(OCTAVE)
    UILAYOUT_PARAM(PORTAMENTO, "Portam")
UILAYOUT_MODULE_END

// ===========================================================================
// PAGE 1  -  Core synth parameters
// ===========================================================================

// -- Left group: Osc1, Osc2, Osc mod, Mixer

UILAYOUT_MODULE(1, PG_OSC1, HORIZ, 1, 0)
    UILAYOUT_PARAM(OSC1P)
    UILAYOUT_PARAM(OSC1_DET)
    UILAYOUT_PARAM(OSC1WAVE)
    UILAYOUT_PARAM(OSC1SH)
UILAYOUT_MODULE_END

UILAYOUT_MODULE(1, PG_OSC2, HORIZ, 1, 1)
    UILAYOUT_PARAM(OSC2P)
    UILAYOUT_PARAM(OSC2_DET)
    UILAYOUT_PARAM(OSC2WAVE)
    UILAYOUT_PARAM(OSC2SH)
UILAYOUT_MODULE_END

UILAYOUT_MODULE(1, PG_OSC_COM, HORIZ, 1, 2)
    UILAYOUT_PARAM(XMOD)
    UILAYOUT_PARAM(OSC2FLTMOD)
    UILAYOUT_PARAM(OSCSYNC_LEVEL)
    UILAYOUT_PARAM(OSC_KEY_SYNC);
UILAYOUT_MODULE_END

UILAYOUT_MODULE(1, PG_MIXER, VERT, 5, 0)
    UILAYOUT_PARAM(OSC1MIX, "Osc1 Level")
    UILAYOUT_PARAM(OSC2MIX, "Osc2 Level")
    UILAYOUT_PARAM(OSC3MIX, "Sub Level")
UILAYOUT_MODULE_END
UILAYOUT_MODULE(1, PG_MIXER, VERT, 5, 3, "Sub Osc")
    UILAYOUT_PARAM(OSC3WAVE, "Wave")
UILAYOUT_MODULE_END

// -- Group 2: Filter, FENV, LENV

UILAYOUT_MODULE(1, PG_FILTER, HORIZ, 6, 0)
    UILAYOUT_PARAM(CUTOFF)
    UILAYOUT_PARAM(RESONANCE)
    UILAYOUT_PARAM(FLT_KF)
    UILAYOUT_PARAM(ENVELOPE_AMT)
UILAYOUT_MODULE_END

UILAYOUT_MODULE(1, PG_FENV, HORIZ, 6, 1)
    UILAYOUT_PARAM(FATK, "Attack")
    UILAYOUT_PARAM(FDEC, "Decay")
    UILAYOUT_PARAM(FSUS, "Sustain")
    UILAYOUT_PARAM(FSUST, "Sust Time")
    UILAYOUT_PARAM(FREL, "Release")
UILAYOUT_MODULE_END

UILAYOUT_MODULE(1, PG_LENV, HORIZ, 6, 2)
    UILAYOUT_PARAM(LATK, "Attack")
    UILAYOUT_PARAM(LDEC, "Decay")
    UILAYOUT_PARAM(LSUS, "Sustain")
    UILAYOUT_PARAM(LSUST, "Sust Time")
    UILAYOUT_PARAM(LREL, "Release")
UILAYOUT_MODULE_END

// -- Group 3: Filter config, VCA

UILAYOUT_MODULE(1, PG_FILTERCFG, HORIZ, 10, 0)
    UILAYOUT_PARAM(RESPONSE)
    UILAYOUT_PARAM(FENV_INVERT)
    UILAYOUT_PARAM(FENV_LINEAR)
UILAYOUT_MODULE_END

UILAYOUT_MODULE(1, PG_VCA, HORIZ, 11, 2, "HPF")
    UILAYOUT_PARAM(HPFFREQ, "Freq")
UILAYOUT_MODULE_END
UILAYOUT_MODULE(1, PG_VCA, HORIZ, 12, 2)
    UILAYOUT_PARAM(VCADRIVE)
    UILAYOUT_PARAM(ENV_MODE, "Env/VCA")
UILAYOUT_MODULE_END

// -- Rows 3-4: LFOs (WIDTH(4), 4 cols x 2 rows = 8 knobs each) ----------

UILAYOUT_MODULE(1, PG_LFO1, WIDTH(4), 1, 3)
    UILAYOUT_PARAM(LFO1FREQ)
    UILAYOUT_PARAM(LFO1WAVE)
    UILAYOUT_PARAM(LFO1DEST)
    UILAYOUT_PARAM(LFO1AMT, "Initial Amt")
    UILAYOUT_PARAM(LFO1_POLARITY)
    UILAYOUT_PARAM(LFO1SYNC)
    UILAYOUT_PARAM(LFO1_AMT_CTRL)
    UILAYOUT_PARAM(LFO1_CONTRAMT, "Contr Amt")
UILAYOUT_MODULE_END

UILAYOUT_MODULE(1, PG_LFO2, WIDTH(4), 6, 3)
    UILAYOUT_PARAM(LFO2FREQ)
    UILAYOUT_PARAM(LFO2WAVE)
    UILAYOUT_PARAM(LFO2DEST)
    UILAYOUT_PARAM(LFO2AMT, "Initial Amt")
    UILAYOUT_PARAM(LFO2_POLARITY)
    UILAYOUT_PARAM(LFO2SYNC)
    UILAYOUT_PARAM(LFO2_AMT_CTRL)
    UILAYOUT_PARAM(LFO2_CONTRAMT, "Contr Amt")
UILAYOUT_MODULE_END

UILAYOUT_MODULE(1, PG_LFO3, WIDTH(4), 10, 3)
    UILAYOUT_PARAM(LFO3FREQ)
    // LFO3WAVE range 0-10: integer values 1-8 are named waveshapes.
    // Angles = value/10 * 270 degrees from knob start.
    UILAYOUT_PARAM_POINTS(
         10, SYM_RSAW,
         67, SYM_TRI,
        125, SYM_SAW,
        145, SYM_PULSE,
        202, SYM_SQUARE,
        260, SYM_RPULSE)
    UILAYOUT_PARAM(LFO3WAVE)
    UILAYOUT_PARAM(LFO3DEST)
    UILAYOUT_PARAM(LFO3AMT, "Initial Amt")
    UILAYOUT_PARAM(LFO3_POLARITY)
    UILAYOUT_PARAM(LFO3SYNC)
    UILAYOUT_PARAM(LFO3_AMT_CTRL)
    UILAYOUT_PARAM(LFO3_CONTRAMT, "Contr Amt")
UILAYOUT_MODULE_END

// ===========================================================================
// PAGE 2  -  Assign / Controllers / Misc
// ===========================================================================

UILAYOUT_MODULE(2, PG_KEYASGN, HORIZ, 1, 0)
    UILAYOUT_PARAM(VOICE_COUNT)
    UILAYOUT_PARAM(ASGN_MODE, "Mode")
    UILAYOUT_PARAM(UNISON_PAN)
    UILAYOUT_PARAM(UNISON_DETUNE)
    UILAYOUT_PARAM(ASGN_RSZ, "Allocation")
    UILAYOUT_PARAM(ASGN_MEM, "Memory")
    UILAYOUT_PARAM(ASGN_ROB)
    UILAYOUT_PARAM(ASGN_RES)
    UILAYOUT_PARAM(ASGN_MTRG)
    UILAYOUT_PARAM(ENV_RST, "Env Attack")
UILAYOUT_MODULE_END

UILAYOUT_MODULE(2, PG_BEND, HORIZ, 1, 1)
    UILAYOUT_PARAM(BENDDEST)
    UILAYOUT_PARAM(BENDRANGE)
UILAYOUT_MODULE_END

UILAYOUT_MODULE(2, PG_CONTR, HORIZ, 3, 1)
    UILAYOUT_PARAM(MODWDEST, "Modw Dest")
    UILAYOUT_PARAM(MODWAMT, "Modw Amt")
    UILAYOUT_PARAM(ATDEST, "Aftert Dest")
    UILAYOUT_PARAM(ATAMT, "Aftert Amt")
UILAYOUT_MODULE_END

UILAYOUT_MODULE(2, PG_CSENS, HORIZ, 7, 1)
    UILAYOUT_PARAM(VEL_SCALE, "Vel Scale")
    UILAYOUT_PARAM(AT_SCALE, "Aftert Scale")
    UILAYOUT_PARAM(VFLTENV, "Fenv Vel")
    UILAYOUT_PARAM(VAMPENV, "Lenv Vel")
UILAYOUT_MODULE_END

UILAYOUT_MODULE(2, PG_SPREAD, HORIZ, 1, 2)
    UILAYOUT_PARAM(UDET, "Oscs")
    UILAYOUT_PARAM(FILTERDER, "Filter")
    UILAYOUT_PARAM(LEVEL_DIF, "Level")
    UILAYOUT_PARAM(PANSPREAD, "Pan")
    UILAYOUT_PARAM(ENVDER, "Envelopes")
    UILAYOUT_PARAM(PORTADER, "Porta")
    UILAYOUT_PARAM(LFOSPREAD, "LFOs")
UILAYOUT_MODULE_END

UILAYOUT_MODULE(2, PG_DSP, HORIZ, 8, 2)
    UILAYOUT_PARAM(OVERSAMPLE, "Ovrsample")
    UILAYOUT_PARAM(ECONOMY_MODE, "Economy")
UILAYOUT_MODULE_END

// -- Clean up --------------------------------------------------------------

#undef UILAYOUT_PAGE
#undef UILAYOUT_MODULE
#undef UILAYOUT_MODULE_END
#undef UILAYOUT_PARAM
#undef UILAYOUT_PARAM_POINTS
