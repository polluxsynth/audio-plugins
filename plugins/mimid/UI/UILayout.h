/*
 * UILayout.h  -  Layout engine for MiMi-d UI
 *
 * Types: ParamWidget, ModuleLayout, PageLayout (top-level, used by UILayoutDefs.h)
 *
 * ButtonStrip class  -  owns strip geometry, draws the button strip.
 *   draw()      -  full redraw, updates cached geometry
 *   geometry()  -  returns cached StripGeometry for hit testing and partial redraws
 *
 * gridToCX / advanceGrid  -  free functions for grid coordinate arithmetic,
 *   used by ButtonStrip and Page.
 *
 * All drawing functions receive a const CairoWidgets reference (wc) which
 * carries UIGeometry and UISettings, eliminating those as separate parameters.
 *
 * Placement:
 *   All knobs live on a uniform grid defined by UIGeometry.
 *   A module's anchor (gridCol, gridRow) gives the grid position of
 *   its first (top-left) knob. Horizontal modules advance along
 *   columns; vertical modules advance along rows.
 *   The frame is drawn to exactly enclose the module's knobs.
 */

/*
 * =========================================================================
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
 * =========================================================================
 */

#pragma once

#include <vector>
#include <string>
#include <cstdio>
#include <cmath>

#include "UIGeometry.h"
#include "CairoWidgets.h"

// -----------------------------------------------------------------------------
// Module width  -  number of columns before wrapping
// -----------------------------------------------------------------------------

// The DIR argument to UILAYOUT_MODULE is an integer column count (width):
//   WIDTH(n)  -  wrap after n columns
//   VERT      -  WIDTH(1): one column, items stack vertically
//   HORIZ     -  WIDTH(16): wide enough to never wrap in practice
// These are defined locally in MiMiUI.cpp around each #include of
// UILayoutDefs.h to avoid polluting other translation units.

// -----------------------------------------------------------------------------
// ParamWidget  -  one knob on the panel
// -----------------------------------------------------------------------------

struct ParamWidget {
    int         paramNo;
    std::string name;
    int         steps;
    float       minVal, maxVal, defVal;
    bool        isInteger;
    std::vector<std::string> scaleLabels;

    // Symbols: pairs of {angleDeg, Symbol}.
    // angleDeg is degrees from knob start (0=bottom-left, 270=bottom-right).
    // Populated by UILAYOUT_PARAM_POINTS in MiMiUI.cpp.
    // Empty for most knobs.
    std::vector<std::pair<float,Symbol>> symPoints;

    std::string formatValue(float val) const
    {
        if (!scaleLabels.empty()) {
            int idx = (int)std::round((val-minVal)/(maxVal-minVal)*(float)steps);
            if (idx < 0) idx = 0;
            if (idx >= (int)scaleLabels.size())
                idx = (int)scaleLabels.size() - 1;
            return scaleLabels[idx];
        } else if (isInteger) {
            return std::to_string((int)std::round((double)val));
        } else {
            char buf[32];
            snprintf(buf, sizeof(buf), "%.1f", (double)val);
            return buf;
        }
    }
};

// -----------------------------------------------------------------------------
// ModuleLayout / PageLayout
// -----------------------------------------------------------------------------

struct ModuleLayout {
    std::string              title;
    int                      width;    // column count before wrapping
    int                      gridCol, gridRow;
    std::vector<ParamWidget> params;
};

struct PageLayout {
    std::string               label;
    std::vector<ModuleLayout> modules;
};

// -----------------------------------------------------------------------------
// StripGeometry  -  cached geometry of button strip dynamic elements.
// Outside any anonymous namespace so ButtonStrip can use it as a member.
// -----------------------------------------------------------------------------

struct StripGeometry {
    float pageBtnX, pageBtnW;
    float dispX, dispY, dispW, dispH;
    float uiBtnX, uiBtnW;   // left edge and width; uiBtnW=-1 before first draw
};

// -----------------------------------------------------------------------------
// ButtonStrip  -  draws the button strip and owns its geometry cache
// -----------------------------------------------------------------------------

class ButtonStrip {
    // uiBtnW=-1: right-anchor, not yet measured
    // uiBtnX=0: anchor not yet set
    StripGeometry fGeom = {
        .pageBtnX = 0, .pageBtnW = 0,
        .dispX = 0, .dispY = 0, .dispW = 0, .dispH = 0,
        .uiBtnX = 0, .uiBtnW = -1.0f
    };
    float         fTitleFontSize  = 0.0f;
    // cached left edge of value field; set whenever the display panel is drawn
    float         fValueX         = 0.0f;

public:
    ButtonStrip() = default;

    // Geometry cache  -  valid after any call to draw()
    const StripGeometry &geometry()      const { return fGeom; }
    float                titleFontSize() const { return fTitleFontSize; }

    // Draw the full button strip. Computes and caches geometry as it renders
    // left-to-right, then draws the display panel last (it needs both the
    // left edge from the page button and the right edge from the UI button).
    void draw(const CairoWidgets &wc,
              bool pageBtnPressed, bool uiBtnPressed,
              float canvasW,
              const char *selName  = nullptr,
              const char *selValue = nullptr,
              RGBA glowColour      = COL_KNOB_GLOW)
    {
        cairo_t *cr = wc.getCR();
        const float stripCY = BTN_STRIP_H / 2.0f;

        // -- "MiMi-d" title -- size directly from strip height --------------
        fTitleFontSize = BTN_STRIP_H * STRIP_TITLE_FONT_RATIO;
        wc.setFont(FONT_NAME_BOLD, fTitleFontSize);
        const float advance = wc.textMeasure(PLUGIN_NAME);
        float titleRightEdge = STRIP_TITLE_X + advance;
        wc.setFillColor(COL_FRAME_TITLE);
        wc.textLeft(STRIP_TITLE_X, stripCY, PLUGIN_NAME);

        // 2x pad each side of the gap
        fGeom.pageBtnX = titleRightEdge + FRAME_TITLE_PAD * 4.0f
                         + STRIP_BTN_GAP + PAGE_BTN_HPAD;

        // -- Vertical break after title ------------------------------------
        drawVBreak(wc, fGeom.pageBtnX - PAGE_BTN_HPAD - STRIP_BTN_GAP);

        // -- Page button -- width measured and cached by drawButton
        // on first call
        wc.drawButton(fGeom.pageBtnX, PAGE_BTN_Y, fGeom.pageBtnW, PAGE_BTN_H,
                      pageBtnPressed, BTN_LABEL_PAGE);

        float afterPageBtn = fGeom.pageBtnX + fGeom.pageBtnW + PAGE_BTN_HPAD;
        fGeom.dispX = afterPageBtn + STRIP_BTN_GAP + PAGE_BTN_HPAD;

        // -- Vertical break after page button ------------------------------
        drawVBreak(wc, afterPageBtn);

        // -- UI button -- right-anchored: set anchor on first call only.
        // drawButton sees w < 0, measures, then stores left edge in uiBtnX
        // and positive width in uiBtnW via references.
        // Anchor is set so the gap to the right of the button equals the gap
        // to the left of it (PAGE_BTN_HPAD + STRIP_BTN_GAP), centering the
        // button between the VBreak line and the canvas right edge.
        if (fGeom.uiBtnX == 0.0f)
            fGeom.uiBtnX = canvasW - PAGE_BTN_HPAD - STRIP_BTN_GAP;

        wc.drawButton(fGeom.uiBtnX, PAGE_BTN_Y, fGeom.uiBtnW, PAGE_BTN_H,
                      uiBtnPressed, BTN_LABEL_UI);

        // -- Vertical break before UI button  (uiBtnX now holds the
        // left edge) --
        drawVBreak(wc, fGeom.uiBtnX - PAGE_BTN_HPAD - STRIP_BTN_GAP);

        // -- Parameter display panel -- drawn last; needs dispX and uiBtnX
        fGeom.dispW = std::min(
            fGeom.uiBtnX - PAGE_BTN_HPAD - STRIP_BTN_GAP
                         - PAGE_BTN_HPAD - fGeom.dispX,
            STRIP_DISP_MAX_W);
        fGeom.dispH = BTN_STRIP_H - 2.0f * PAGE_BTN_Y;
        fGeom.dispY = PAGE_BTN_Y;

        if (fGeom.dispW > STRIP_DISP_MIN_W) {
            wc.clearDisplayArea(fGeom.dispX, fGeom.dispY,
                                fGeom.dispW, fGeom.dispH);
            wc.drawParamDisplayFrame(fGeom.dispX, fGeom.dispY,
                                     fGeom.dispW, fGeom.dispH);
            fValueX = wc.drawParamText(fGeom.dispX, fGeom.dispY,
                                       fGeom.dispW, fGeom.dispH,
                                       selName, selValue, glowColour);
        }

        // -- Horizontal separator below strip ------------------------------
        // Engraved effect: dark line + 1px-offset light highlight line.
        const float y = BTN_STRIP_SEP_Y;
        cairo_new_path(cr);
        cairo_move_to(cr, 0.0f, y);
        cairo_line_to(cr, canvasW, y);
        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, STRIP_SEP_DARK_A);
        cairo_set_line_width(cr, 1.0f);
        cairo_stroke(cr);
        cairo_new_path(cr);
        cairo_move_to(cr, 0.0f, y + ENGRAVE_OFFSET);
        cairo_line_to(cr, canvasW, y + ENGRAVE_OFFSET);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, STRIP_SEP_LIGHT_A);
        cairo_set_line_width(cr, 1.0f);
        cairo_stroke(cr);
    }

    // Partial redraws  -  require a prior draw() to have cached fGeom.
    // drawDisplay: full panel repaint (chrome + name + value); also caches
    //   fValueX for subsequent drawValue calls.
    // drawValue: repaints only the value field using cached fValueX - the
    //   name text and chrome already on the surface are left untouched.
    void drawDisplay(const CairoWidgets &wc,
                     const char *selName, const char *selValue,
                     RGBA glowColour = COL_KNOB_GLOW)
    {
        if (fGeom.dispW <= STRIP_DISP_MIN_W) return;
        wc.clearDisplayArea(fGeom.dispX, fGeom.dispY,
                            fGeom.dispW, fGeom.dispH);
        wc.drawParamDisplayFrame(fGeom.dispX, fGeom.dispY,
                                 fGeom.dispW, fGeom.dispH);
        fValueX = wc.drawParamText(fGeom.dispX, fGeom.dispY,
                                   fGeom.dispW, fGeom.dispH,
                                   selName, selValue, glowColour);
    }

    // selValue may be null/empty (no param selected); in that case only the
    // clear runs, leaving the value field blank.
    void drawValue(const CairoWidgets &wc,
                   const char *selValue,
                   RGBA glowColour = COL_KNOB_GLOW) const
    {
        if (fGeom.dispW <= STRIP_DISP_MIN_W || fValueX == 0.0f) return;
        float width = fGeom.dispW - (fValueX - fGeom.dispX);
        wc.clearDisplayValueArea(fValueX, fGeom.dispY, width, fGeom.dispH);
        if (!selValue || selValue[0] == '\0') return;  // no value text to draw
        wc.drawParamValue(fGeom.dispX, fGeom.dispY, fGeom.dispW, fGeom.dispH,
                          fValueX, selValue, glowColour);
    }

private:
    // Draw engraved vertical break (dark + light lines) at x
    static void drawVBreak(const CairoWidgets &wc, float x)
    {
        cairo_t *cr = wc.getCR();
        cairo_new_path(cr);
        cairo_move_to(cr, x, 0.0f);
        cairo_line_to(cr, x, BTN_STRIP_SEP_Y);
        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, STRIP_SEP_DARK_A);
        cairo_set_line_width(cr, FRAME_LINE_W);
        cairo_stroke(cr);
        cairo_new_path(cr);
        cairo_move_to(cr, x + ENGRAVE_OFFSET, 0.0f);
        cairo_line_to(cr, x + ENGRAVE_OFFSET, BTN_STRIP_SEP_Y);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, STRIP_SEP_LIGHT_A);
        cairo_set_line_width(cr, FRAME_LINE_W);
        cairo_stroke(cr);
    }
};

// -----------------------------------------------------------------------------
// Grid helpers  -  used by ButtonStrip and Page
// -----------------------------------------------------------------------------

inline void gridToCX(int col, int row, const GridOrigin &orig,
                     const UIGeometry &g, float &cx, float &cy)
{
    cx = orig.x + col * g.kpitch;
    cy = orig.y + row * g.rowPitch;
}

inline void advanceGrid(int cols, int baseCol,
                        int &col, int &row, int &slotInRow)
{
    slotInRow++;
    if (slotInRow >= cols) { slotInRow = 0; col = baseCol; row++; }
    else                   { col++; }
}
