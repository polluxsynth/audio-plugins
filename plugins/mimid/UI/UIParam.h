/*
 * UIParam.h  -  Per-parameter metadata for MiMi-d UI
 *
 * ParamMeta holds the engine-side information the UI needs about a parameter:
 * its name, range, and scale points. This struct is pure read-only data after
 * construction (buildPages() may write uiName overrides during startup).
 *
 * Canvas position and layout data (cx, cy, pw, pageIndex) live separately in
 * Page::fKnobLayout[], populated by Page::buildKnobInfo() at startup.
 *
 * ParamTables is populated once by buildMetaTables() and owned by MiMiUI as
 * fParamTables.  Consumers bind named references (fParamMeta, fScaleLabels)
 * directly into it rather than passing the struct around as a whole.
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

#pragma once

#include <vector>
#include <string>
#include "Engine/ParamsEnum.h"  // PARAM_COUNT, SP_COUNT, SP_NONE, PG_COUNT
#include "UI/UILayout.h"        // ParamWidget

struct ParamMeta {
    const char *name;       // canonical name from ParamDefs.h
    const char *uiName;     // display name  -  same as name unless overridden
    const char *groupName;  // group title from PARAMGROUP, or nullptr
    int         spId;       // SP_* id  -  indexes into scaleLabels
    int         steps;      // 0 = continuous, >0 = stepped (number of intervals)
    float       min, max, def;
};

// Owns the arrays populated by buildMetaTables().
// MiMiUI holds one as fParamTables; consumers bind named references into it.
struct ParamTables {
    ParamMeta                paramMeta[PARAM_COUNT];
    std::vector<std::string> scaleLabels[SP_COUNT];
    const char              *groupName[PG_COUNT];  // for buildPages() title fallback
};
