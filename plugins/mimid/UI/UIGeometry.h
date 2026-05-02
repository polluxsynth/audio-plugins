/*
 * UIGeometry.h  -  Derived geometry for MiMi-d UI
 *
 * All values computed from UIConstants.h. Nothing here needs editing;
 * change the constants and these update automatically.
 *
 * UIGeometry is a plain struct, computed once at startup and passed
 * by const reference to all drawing functions.
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

#include "UIConstants.h"
#include <cmath>

// Canvas position of the knob grid origin (col 0, row 0 knob centre).
// Defined here because UIGeometry::gridOrigin() returns it.
struct GridOrigin {
    float x;   // x of grid column 0 knob centre
    float y;   // y of grid row 0 knob centre
};

struct UIGeometry {
    // -- Knob -------------------------------------------------------------
    float r;           // knob body radius
    float tw;          // track stroke width
    float trackR;      // radius of arc track centre line
    float kpitch;      // knob centre-to-centre (horizontal)

    // -- Font -------------------------------------------------------------
    float fontSize;        // value text font size
    float labelFontSize;   // parameter name and frame title font size

    // -- Name label -------------------------------------------------------
    float nameH;       // height of name label row
    float nameDY;      // distance from knob centre to top of name text

    // -- Frame (asymmetric) -----------------------------------------------
    float topHalf;     // knob centre -> top frame line
    float botHalf;     // knob centre -> bottom frame line

    // -- Vertical row grid ------------------------------------------------
    float rowPitch;    // centre-to-centre vertical distance between knob rows

    // -- Knob grid origin -------------------------------------------------
    GridOrigin origin; // canvas position of grid (col 0, row 0) knob centre

    // -- Arc angles (radians) ---------------------------------------------
    float arcStart;    // start angle (lower-left)
    float arcRange;    // total arc sweep

    // -- Constructor: compute everything from constants --------------------
    UIGeometry()
    {
        r       = KNOB_R;
        tw      = TRACK_W;
        trackR  = r + tw * TRACK_R_MULT;
        kpitch  = 2.0f * trackR + KNOB_PITCH_EXTRA;

        fontSize      = std::max(FONT_SIZE_MIN, r * FONT_SIZE_RATIO);
        labelFontSize = fontSize * LABEL_FONT_SCALE;
        nameH    = labelFontSize + NAME_H_PAD;
        nameDY   = trackR + NAME_DY_OFFSET;

        topHalf  = trackR + FRAME_VPAD_TOP;
        botHalf  = nameDY + nameH + FRAME_VPAD_BOT;
        const float fh = topHalf + botHalf;

        rowPitch = fh + FRAME_GAP_V;

        origin.x = kpitch / 2.0f + GRID_ORIGIN_MARGIN;
        origin.y = BTN_STRIP_H + GRID_ORIGIN_MARGIN + topHalf;

        arcStart = ARC_START_DEG * (float)M_PI / 180.0f;
        arcRange = ARC_RANGE_DEG * (float)M_PI / 180.0f;
    }

    // -- Helpers -----------------------------------------------------------

    // Angle for a normalised parameter value [0..1]
    float valueToAngle(float norm) const {
        return arcStart + norm * arcRange;
    }

    // Frame top y given knob centre y
    float frameTop(float cy) const { return cy - topHalf; }

    // Frame bottom y given knob centre y
    float frameBot(float cy) const { return cy + botHalf; }

    // Canvas position of the knob grid origin (col 0, row 0 knob centre)
    const GridOrigin &gridOrigin() const { return origin; }
};
