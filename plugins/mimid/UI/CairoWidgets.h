/*
 * CairoWidgets.h  -  Cairo drawing functions for MiMi-d UI
 *
 * Widget library for MiMi-d using Cairo + FreeType.  Mirrors the
 * structure and public API of now-defunct NanoWidgets.h exactly so
 * that all callers (UIMenu, UILayout, UIPage) need only a mechanical
 * substitution of the type name. Note that Cairo and nanoVG calculate
 * font sizes differently, so changing between the two requires adjustments
 * of various font constants if consistent appearance is to be maintained.
 *   - Call setCR(cr) at the start of each frame before drawing
 *   - getCR() exposes the Cairo context for callers that need it
 *     directly (UIMenu, UILayout)
 *   - Fonts are owned by CairoWidgets; FreeType state is initialised
 *     in the constructor from the embedded NotoSans header arrays.
 *   - Knob face uses a radial gradient with flat-zone + bevel.
 *   - Knob shadow is rendered as a squashed filled ellipse.
 *   - Knob dome adds a centre-bright radial white gloss overlay.
 *   - Cairo renders directly to a cairo_surface_t every frame.
 *
 * Build: DISTRHO_UI_USE_CAIRO=1, link against cairo and freetype2.
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
#include <cairo/cairo.h>
#include <cairo/cairo-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cmath>
#include <string>
#include <cstdio>
#include <vector>
#include "UIConstants.h"
#include "UIGeometry.h"
#include "UISettings.h"
#include "NotoSans-Regular.h"
#include "NotoSans-Bold.h"

// ---------------------------------------------------------------------------
// CairoWidgets
// ---------------------------------------------------------------------------

class CairoWidgets {
    cairo_t           *fCR        = nullptr;
    UIGeometry         fGeometry;
    UISettings         fSettings;

    // FreeType / Cairo font state  -  owned for the widget lifetime
    FT_Library         fFTLib     = nullptr;
    FT_Face            fFTRegular = nullptr;
    FT_Face            fFTBold    = nullptr;
    cairo_font_face_t *fFaceReg   = nullptr;
    cairo_font_face_t *fFaceBold  = nullptr;

public:
    CairoWidgets() { initFonts(); }

    CairoWidgets(const UIGeometry &g, const UISettings &s)
        : fGeometry(g), fSettings(s) { initFonts(); }

    ~CairoWidgets()
    {
        if (fFaceReg)   cairo_font_face_destroy(fFaceReg);
        if (fFaceBold)  cairo_font_face_destroy(fFaceBold);
        if (fFTRegular) FT_Done_Face(fFTRegular);
        if (fFTBold)    FT_Done_Face(fFTBold);
        if (fFTLib)     FT_Done_FreeType(fFTLib);
    }

    // Non-copyable: owns FreeType resources
    CairoWidgets(const CairoWidgets &)            = delete;
    CairoWidgets &operator=(const CairoWidgets &) = delete;

    // -- Context --------------------------------------------------------------

    // Call at the start of each frame before any drawing.
    void setCR(cairo_t *cr) { fCR = cr; }

    // Expose context for callers needing it directly (UIMenu, UILayout).
    cairo_t *getCR() const  { return fCR; }

    void updateSettings(const UISettings &s) { fSettings = s; }

    const UIGeometry &geometry() const { return fGeometry; }
    const UISettings &settings() const { return fSettings; }

    // -- Low-level helpers (also used by UIMenu/UILayout) --------------------

    void setFillColor(const RGBA &c) const
    {
        cairo_set_source_rgba(fCR, c.r, c.g, c.b, c.a);
    }

    // In Cairo stroke and fill share one source; set it here, distinguish
    // at cairo_stroke() / cairo_fill() time.
    void setStrokeColor(const RGBA &c) const
    {
        cairo_set_source_rgba(fCR, c.r, c.g, c.b, c.a);
    }

    // Horizontally and vertically centred text at (cx, cy).
    // Returns the x position after the last glyph (x_advance from origin).
    float textCentred(float cx, float cy, const char *txt) const
    {
        cairo_text_extents_t e;
        cairo_text_extents(fCR, txt, &e);
        cairo_font_extents_t fe;
        cairo_font_extents(fCR, &fe);
        const float tx = cx - e.width / 2.0 - e.x_bearing;
        const float ty = cy + fe.ascent / 2.0 - fe.descent / 2.0;
        cairo_move_to(fCR, tx, ty);
        cairo_show_text(fCR, txt);
        return (tx + e.x_advance);
    }

    // Left-aligned, vertically centred text.
    // Returns the x position after the last glyph.
    float textLeft(float x, float y, const char *txt) const
    {
        cairo_text_extents_t e;
        cairo_text_extents(fCR, txt, &e);
        cairo_font_extents_t fe;
        cairo_font_extents(fCR, &fe);
        const float tx = x - e.x_bearing;
        const float ty = y + fe.ascent / 2.0 - fe.descent / 2.0;
        cairo_move_to(fCR, tx, ty);
        cairo_show_text(fCR, txt);
        return tx + e.x_advance;
    }

    // Horizontally centred, top-aligned text at (cx, y).
    // Returns the x position after the last glyph.
    float textTop(float cx, float y, const char *txt) const
    {
        cairo_text_extents_t e;
        cairo_text_extents(fCR, txt, &e);
        const float tx = cx - e.width / 2.0 - e.x_bearing;
        cairo_font_extents_t fe;
        cairo_font_extents(fCR, &fe);
        // Top-aligned: baseline = y + ascent
        cairo_move_to(fCR, tx, y + fe.ascent);
        cairo_show_text(fCR, txt);
        return tx + e.x_advance;
    }

    // Measure text advance width without drawing.
    // Font and size must be set beforehand.
    float textMeasure(const char *txt) const
    {
        cairo_text_extents_t e;
        cairo_text_extents(fCR, txt, &e);
        return e.x_advance;
    }

    // Select font face by role name (FONT_NAME_REGULAR / FONT_NAME_BOLD).
    void setFont(const char *name, float size) const
    {
        cairo_font_face_t *face =
            (name && name[0] == 'b') ? fFaceBold : fFaceReg;
        if (face)
            cairo_set_font_face(fCR, face);
        cairo_set_font_size(fCR, size);
    }

    // -- Public drawing interface ---------------------------------------------

    // -- Symbols --------------------------------------------------------------

    // Draw one symbol centred at (cx, cy), scaled by s.
    // Waveform geometry is governed by SYM_HW (half-width), SYM_HH
    // (half-height), and SYM_PW (pulse width fraction), in UIConstants.h.
    void drawSymbol(Symbol symbol, float cx, float cy, float s) const
    {
        if (symbol == SYM_NONE)
            return;

        // SYM_INF: two 270-degree arcs (lobes) joined by crossed lines.
        // Each lobe has a 90-degree gap facing inward; two diagonal lines
        // cross the centre connecting the open endpoints.
        // Handled separately from the polyline-based symbols below.
        if (symbol == SYM_INF) {
            const float r  = s * SYM_INF_R;
            const float ox = s * SYM_INF_OX;
            // Endpoint offset from lobe centre at 45 deg: r * cos(45)
            const float e  = r * 0.7071;
            // Maths convention angles (Cairo arc: CCW / maths positive).
            // In screen space (y down) this renders as CW visually.
            const float a45  = M_PI * 1.0 / 4.0;
            const float a135 = M_PI * 3.0 / 4.0;
            const float a225 = M_PI * 5.0 / 4.0;
            const float a315 = M_PI * 7.0 / 4.0;
            cairo_set_line_width(fCR, SYM_LW);
            cairo_set_line_cap(fCR, CAIRO_LINE_CAP_ROUND);
            cairo_set_line_join(fCR, CAIRO_LINE_JOIN_ROUND);
            // Left lobe: centre (cx-ox, cy), gap faces right.
            // Arc: 45 deg to 315 deg increasing = 270 deg CW on screen.
            cairo_new_path(fCR);
            cairo_arc(fCR, cx - ox, cy, r, a45, a315);
            cairo_stroke(fCR);
            // Right lobe: centre (cx+ox, cy), gap faces left.
            // Arc: 225 deg to 135 deg increasing = 270 deg CW on screen.
            cairo_new_path(fCR);
            cairo_arc(fCR, cx + ox, cy, r, a225, a135);
            cairo_stroke(fCR);
            // Crossed lines joining the open endpoints.
            cairo_new_path(fCR);
            cairo_move_to(fCR, cx - ox + e, cy - e);
            cairo_line_to(fCR, cx + ox - e, cy + e);
            cairo_stroke(fCR);
            cairo_new_path(fCR);
            cairo_move_to(fCR, cx - ox + e, cy + e);
            cairo_line_to(fCR, cx + ox - e, cy - e);
            cairo_stroke(fCR);
            return;
        }

        const float hw = s * SYM_HW;
        const float hh = s * SYM_HH;
        const float x0 = cx - hw;
        const float x1 = cx + hw;
        const float y0 = cy - hh;
        const float y1 = cy + hh;
        const float mx = cx;
        const float my = cy;
        const float pw = s * SYM_PW;

        cairo_new_path(fCR);
        switch (symbol) {
            case SYM_SAW:
                cairo_move_to(fCR, x0, y1);
                cairo_line_to(fCR, x1, y0);
                cairo_line_to(fCR, x1, y1);
                break;
            case SYM_RSAW:
                cairo_move_to(fCR, x0, y1);
                cairo_line_to(fCR, x0, y0);
                cairo_line_to(fCR, x1, y1);
                break;
            case SYM_TRI:
                cairo_move_to(fCR, x0, y1);
                cairo_line_to(fCR, mx, y0);
                cairo_line_to(fCR, x1, y1);
                break;
            case SYM_SQUARE:
                cairo_move_to(fCR, x0, my);
                cairo_line_to(fCR, x0, y0);
                cairo_line_to(fCR, mx, y0);
                cairo_line_to(fCR, mx, y1);
                cairo_line_to(fCR, x1, y1);
                cairo_line_to(fCR, x1, my);
                break;
            case SYM_PULSE:
                cairo_move_to(fCR, x0,      y1);
                cairo_line_to(fCR, x0,      y0);
                cairo_line_to(fCR, x0 + pw, y0);
                cairo_line_to(fCR, x0 + pw, y1);
                cairo_line_to(fCR, x1,      y1);
                break;
            case SYM_RPULSE:
                cairo_move_to(fCR, x0,      y0);
                cairo_line_to(fCR, x1 - pw, y0);
                cairo_line_to(fCR, x1 - pw, y1);
                cairo_line_to(fCR, x1,      y1);
                cairo_line_to(fCR, x1,      y0);
                break;
            default:
                return;
        }
        cairo_set_line_width(fCR, SYM_LW);
        cairo_set_line_cap(fCR, CAIRO_LINE_CAP_SQUARE);
        cairo_set_line_join(fCR, CAIRO_LINE_JOIN_MITER);
        cairo_stroke(fCR);
    }

    // Draw all symbols for a knob.  Each entry is a {angleDeg, symbol}
    // pair where angleDeg is degrees from the knob arc start (0 =
    // bottom-left, 270 = bottom-right).  Symbol centres are placed at
    // trackOuter + SYM_DIST + SYM_HH so the inner symbol edge clears
    // the outermost tick stroke.
    void drawSymbols(
        float cx, float cy,
        const std::vector<std::pair<float,Symbol>> &points) const
    {
        if (points.empty())
            return;
        const float trackOuter = fGeometry.trackR
                               + fGeometry.tw * TICK_STEP_OUTER
                               + TICK_STEP_WIDTH * 0.5f;
        const float hh         = SYM_SIZE * SYM_HH;
        const float symCentreR = trackOuter + SYM_DIST + hh;
        const float startRad   =
            ARC_START_DEG * static_cast<float>(M_PI) / 180.0f;

        // SYM_Y_NUDGE shifts symbol baseline so label text and symbols
        // appear optically aligned.
        const float adjCY = cy + SYM_Y_NUDGE;
        setStrokeColor(COL_KNOB_LABEL);
        for (const auto &pt : points) {
            const float angle =
                startRad + pt.first
                         * static_cast<float>(M_PI) / 180.0f;
            float sx = cx + std::cos(angle) * symCentreR;
            float sy = adjCY + std::sin(angle) * symCentreR;
            // SYM_INF is drawn relative to its lobe centres, not its
            // overall bounding centre; nudge it radially so the inner
            // lobe edge sits at the same distance as other symbols.
            if (pt.second == SYM_INF) {
                const float nudge = SYM_SIZE * SYM_INF_OX;
                sx += std::cos(angle) * nudge;
            }
            drawSymbol(pt.second, sx, sy, SYM_SIZE);
        }
    }

    void drawBackground(float w, float h) const
    {
        setFillColor(COL_BACKGROUND);
        cairo_rectangle(fCR, 0, 0, w, h);
        cairo_fill(fCR);
    }

    // Clear the bounding area of one knob back to the background colour.
    // Used before partial (knob-only) redraws.  A clip region constrains
    // painting to a rectangle whose bottom edge aligns with the knob
    // equator + half track width, protecting the name label below.
    void clearKnobArea(float cx, float cy) const
    {
        const float tickReach = fGeometry.trackR
                              + fGeometry.tw * TICK_STEP_OUTER
                              + TICK_STEP_WIDTH * 0.25f;
        const float clipBottom = cy + fGeometry.trackR
                               + fGeometry.tw * 0.5f;
        cairo_save(fCR);
        cairo_rectangle(fCR,
            cx - tickReach, cy - tickReach,
            tickReach * 2.0f, clipBottom - (cy - tickReach));
        cairo_clip(fCR);
        cairo_arc(fCR, cx, cy, tickReach, 0, 2.0 * M_PI);
        setFillColor(COL_BACKGROUND);
        cairo_fill(fCR);
        cairo_restore(fCR);
    }

    // showIndex, showArc, showValue are taken from fSettings.
    // glowColour varies by page and is supplied by the caller.
    void drawKnob(float cx, float cy,
                  float norm, int steps, bool isInteger,
                  const char *label, const char *valueTxt,
                  float zeroNorm    = -1.0f,
                  bool selected     = false,
                  bool drawName     = true,
                  RGBA glowColour   = COL_KNOB_GLOW) const
    {
        const UIGeometry &g   = fGeometry;
        const float       r   = g.r;
        const float       tR  = g.trackR;
        const float       tw  = g.tw;
        const float       angle = g.valueToAngle(norm);

        // -- Shadow ----------------------------------------------------------
        // Squashed filled ellipse, offset from knob centre.
        if (KNOB_SHADOW_BLUR > 0.0f) {
            cairo_save(fCR);
            cairo_translate(fCR,
                            cx + KNOB_SHADOW_BLUR * KNOB_SHADOW_OFFSET_X,
                            cy + KNOB_SHADOW_BLUR * KNOB_SHADOW_OFFSET_Y);
            cairo_scale(fCR, 1.0, KNOB_SHADOW_SQUASH);
            cairo_arc(fCR, 0, 0,
                      r + KNOB_SHADOW_RADIUS, 0, 2.0 * M_PI);
            cairo_set_source_rgba(fCR, 0, 0, 0, KNOB_SHADOW_ALPHA);
            cairo_fill(fCR);
            cairo_restore(fCR);
        }

        // -- Track background ------------------------------------------------
        if (fSettings.showArc) {
            cairo_arc(fCR, cx, cy, tR,
                      g.arcStart, g.arcStart + g.arcRange);
            setStrokeColor(COL_TRACK_BG);
            cairo_set_line_width(fCR, tw);
            cairo_set_line_cap(fCR, CAIRO_LINE_CAP_ROUND);
            cairo_stroke(fCR);
        }

        // -- Track fill ------------------------------------------------------
        if (fSettings.showArc) {
            if (zeroNorm >= 0.0f) {
                const float za = g.valueToAngle(zeroNorm);
                if (norm > zeroNorm) {
                    cairo_arc(fCR, cx, cy, tR, za, angle);
                    setStrokeColor(COL_TRACK_FILL);
                    cairo_set_line_width(fCR, tw);
                    cairo_set_line_cap(fCR, CAIRO_LINE_CAP_ROUND);
                    cairo_stroke(fCR);
                } else if (norm < zeroNorm) {
                    cairo_arc_negative(fCR, cx, cy, tR, za, angle);
                    setStrokeColor(COL_TRACK_FILL);
                    cairo_set_line_width(fCR, tw);
                    cairo_set_line_cap(fCR, CAIRO_LINE_CAP_ROUND);
                    cairo_stroke(fCR);
                }
            } else {
                // 0.002 threshold avoids a tiny stub at minimum value
                if (norm > 0.002f) {
                    cairo_arc(fCR, cx, cy, tR, g.arcStart, angle);
                    setStrokeColor(COL_TRACK_FILL);
                    cairo_set_line_width(fCR, tw);
                    cairo_set_line_cap(fCR, CAIRO_LINE_CAP_ROUND);
                    cairo_stroke(fCR);
                }
            }
        }

        // -- Ticks -----------------------------------------------------------
        // Bipolar zero tick  -  always drawn
        if (zeroNorm >= 0.0f)
            drawTick(cx, cy, tR, g.valueToAngle(zeroNorm), true, false);

        // End ticks  -  always drawn
        drawTick(cx, cy, tR, g.arcStart,              true, false);
        drawTick(cx, cy, tR, g.arcStart + g.arcRange, true, false);

        // Step ticks
        if (steps > 1) {
            for (int i = 0; i <= steps; i++)
                drawTick(cx, cy, tR,
                         g.valueToAngle((float)i / (float)steps),
                         false, false);
        }

        // -- Rim stroke ------------------------------------------------------
        cairo_arc(fCR, cx, cy, r, 0, 2.0 * M_PI);
        setStrokeColor(COL_KNOB_RIM);
        cairo_set_line_width(fCR, KNOB_RIM_W);
        cairo_stroke(fCR);

        // -- Face ------------------------------------------------------------
        // Radial gradient: flat bright centre out to KNOB_FLAT_ZONE,
        // then a sharp bevel transition at KNOB_FLAT_ZONE + epsilon,
        // darkening to rim.
        {
            const RGBA &base = COL_KNOB_BASE;
            const RGBA  cen  = lighten(base, KNOB_CENTRE_LIFT);
            const RGBA  rim  = darken(base,  KNOB_RIM_DARKEN);
            cairo_pattern_t *p =
                cairo_pattern_create_radial(cx, cy, 0, cx, cy, r);
            cairo_pattern_add_color_stop_rgba(p, 0.0,
                                              cen.r, cen.g, cen.b, 1.0);
            cairo_pattern_add_color_stop_rgba(p, KNOB_FLAT_ZONE,
                                              cen.r, cen.g, cen.b, 1.0);
            // Sharp bevel just outside the flat zone
            cairo_pattern_add_color_stop_rgba(p, KNOB_FLAT_ZONE + 0.01,
                                              base.r, base.g, base.b, 1.0);
            cairo_pattern_add_color_stop_rgba(p, 1.0,
                                              rim.r, rim.g, rim.b, 1.0);
            cairo_arc(fCR, cx, cy, r, 0, 2.0 * M_PI);
            cairo_set_source(fCR, p);
            cairo_fill(fCR);
            cairo_pattern_destroy(p);
        }

        // -- Dome ------------------------------------------------------------
        // Radial white overlay: centre-bright gloss fading to transparent
        // at the knob edge.
        {
            cairo_pattern_t *p =
                cairo_pattern_create_radial(cx, cy, 0, cx, cy, r);
            cairo_pattern_add_color_stop_rgba(p, 0.0,
                                              1.0, 1.0, 1.0,
                                              KNOB_DOME_ALPHA_CTR);
            cairo_pattern_add_color_stop_rgba(p, KNOB_DOME_MID_POS,
                                              1.0, 1.0, 1.0,
                                              KNOB_DOME_ALPHA_MID);
            cairo_pattern_add_color_stop_rgba(p, 1.0,
                                              1.0, 1.0, 1.0,
                                              0.0);
            cairo_arc(fCR, cx, cy, r, 0, 2.0 * M_PI);
            cairo_set_source(fCR, p);
            cairo_fill(fCR);
            cairo_pattern_destroy(p);
        }

        // -- Specular --------------------------------------------------------
        // Top-edge linear gradient fading downward, clipped to face circle.
        {
            cairo_pattern_t *p =
                cairo_pattern_create_linear(cx, cy - r,
                                            cx, cy - r * KNOB_SPEC_FADE_Y);
            cairo_pattern_add_color_stop_rgba(p, 0.0,
                                              1, 1, 1,
                                              KNOB_SPEC_OPACITY);
            cairo_pattern_add_color_stop_rgba(p, 1.0,
                                              1, 1, 1,
                                              0.0);
            // Slightly smaller radius to avoid aliasing at rim edge
            cairo_arc(fCR, cx, cy, r * KNOB_SPEC_RADIUS,
                      0, 2.0 * M_PI);
            cairo_set_source(fCR, p);
            cairo_fill(fCR);
            cairo_pattern_destroy(p);
        }

        // -- Indicator line --------------------------------------------------
        if (fSettings.showIndex) {
            const float i0 = r * IND_R_INNER;
            const float i1 = r * IND_R_OUTER;
            const float lw = std::max(IND_WIDTH_MIN, r * IND_WIDTH_RATIO);
            cairo_move_to(fCR, cx + std::cos(angle) * i0,
                               cy + std::sin(angle) * i0);
            cairo_line_to(fCR, cx + std::cos(angle) * i1,
                               cy + std::sin(angle) * i1);
            setStrokeColor(COL_INDICATOR);
            cairo_set_line_width(fCR, lw);
            cairo_set_line_cap(fCR, CAIRO_LINE_CAP_ROUND);
            cairo_stroke(fCR);
        }

        // -- Selection glow --------------------------------------------------
        // Radial gradient fill in the annular region between knob edge and
        // arc track outer edge.
        if (selected) {
            const float glowOuter = tR + tw * 0.5;
            cairo_pattern_t *gp =
                cairo_pattern_create_radial(cx, cy, r, cx, cy, glowOuter);
            cairo_pattern_add_color_stop_rgba(gp, 0.0,
                                              glowColour.r, glowColour.g, glowColour.b,
                                              0.0);
            cairo_pattern_add_color_stop_rgba(gp, KNOB_GLOW_PEAK_POS,
                                              glowColour.r, glowColour.g, glowColour.b,
                                              KNOB_GLOW_PEAK_ALPHA);
            cairo_pattern_add_color_stop_rgba(gp, 1.0,
                                              glowColour.r, glowColour.g, glowColour.b,
                                              0.0);
            cairo_arc(fCR, cx, cy, glowOuter, 0, 2.0 * M_PI);
            cairo_set_source(fCR, gp);
            cairo_fill(fCR);
            cairo_pattern_destroy(gp);
        }

        // -- Value text (centred on knob face) --------------------------------
        const bool drawValueText =
            (fSettings.showValue == SHOW_ALL) ||
            (fSettings.showValue == SHOW_STEPPED &&
             (steps > 0 || isInteger));
        if (drawValueText) {
            setFont(FONT_NAME_BOLD, g.fontSize);
            setFillColor(COL_VALUE_TEXT);
            textCentred(cx, cy, valueTxt);
        }

        // -- Name label (below knob) -----------------------------------------
        if (drawName) {
            setFont(FONT_NAME_REGULAR, g.labelFontSize);
            setFillColor(COL_NAME_TEXT);
            textTop(cx, cy + g.nameDY, label);
        }
    }

    void drawGroupFrame(float fx, float fy, float fw, float fh,
                        const char *title) const
    {
        const UIGeometry &g  = fGeometry;
        const float       rr = FRAME_CORNER_R;
        const float       lx = fx + FRAME_TITLE_X;
        const float     lpad = FRAME_TITLE_PAD;

        // Draw title first; textLeft() returns the x advance after the
        // last glyph, giving the gap end without a separate measurement.
        setFont(FONT_NAME_BOLD, g.labelFontSize);
        setFillColor(COL_FRAME_TITLE);
        const float textEndX = textLeft(lx + lpad, fy, title);
        const float gapW    = (textEndX - lx) + lpad;

        // Frame path with gap at top-left for the title text.
        cairo_new_path(fCR);
        cairo_move_to(fCR, lx + gapW, fy);
        cairo_arc(fCR, fx + fw - rr, fy + rr,      rr, -M_PI / 2.0, 0);
        cairo_arc(fCR, fx + fw - rr, fy + fh - rr, rr,  0,          M_PI / 2.0);
        cairo_arc(fCR, fx + rr,      fy + fh - rr, rr,  M_PI / 2.0, M_PI);
        cairo_arc(fCR, fx + rr,      fy + rr,      rr,  M_PI,       3.0 * M_PI / 2.0);
        cairo_line_to(fCR, lx, fy);
        setStrokeColor(COL_FRAME_LINE);
        cairo_set_line_width(fCR, FRAME_LINE_W);
        cairo_stroke(fCR);
    }

    // -- Parameter display panel ------------------------------------------
    //
    //   clearDisplayArea        - reset panel area to display background
    //   clearDisplayValueArea   - reset value field only (keeps name)
    //   drawParamDisplayFrame   - frame: border + inner shadow
    //   drawParamText           - draw name + value; returns valueX
    //   drawParamValue          - draw value at a pre-computed valueX
    //
    // For partial redraws (value only): call clearDisplayValueArea then
    // drawParamValue.

    void clearDisplayArea(float x, float y, float w, float h) const
    {
        rrect(x, y, w, h, DISP_CORNER_R);
        cairoSetDisplayBg();
        cairo_fill(fCR);
    }

    void drawParamDisplayFrame(float x, float y,
                               float w, float h) const
    {
        // Border  -  inset by half stroke so it stays inside the panel
        rrect(x + 0.5f, y + 0.5f,
              w - DISP_BORDER_STROKE_W, h - DISP_BORDER_STROKE_W,
              DISP_CORNER_R - 0.5f);
        cairo_set_source_rgba(fCR, 0.0, 0.0, 0.0, DISP_BORDER_A);
        cairo_set_line_width(fCR, DISP_BORDER_STROKE_W);
        cairo_stroke(fCR);

        // Inner shadow  -  dark line just inside the top edge
        cairo_move_to(fCR, x + DISP_CORNER_R,     y + DISP_SHADOW_Y);
        cairo_line_to(fCR, x + w - DISP_CORNER_R, y + DISP_SHADOW_Y);
        cairo_set_source_rgba(fCR, 0.0, 0.0, 0.0, DISP_SHADOW_A);
        cairo_set_line_width(fCR, DISP_SHADOW_STROKE_W);
        cairo_stroke(fCR);
    }

    // Clear the value field only (keeps the name text intact).
    // Uses a plain rect inset by the border stroke width so the
    // frame lines are not overdrawn.
    void clearDisplayValueArea(float x, float y,
                               float w, float h) const
    {
        const float inset = DISP_BORDER_STROKE_W * 2.0f;
        cairo_rectangle(fCR, x,         y + inset,
                             w - inset, h - inset * 2.0f);
        cairoSetDisplayBg();
        cairo_fill(fCR);
    }

    // Draw name + value in the display panel.
    // Clips text to the panel interior to prevent overdraw at the edges.
    // Returns valueX (left edge of value field) for the caller to cache.
    float drawParamText(float x, float y, float w, float h,
                        const char *name, const char *value,
                        RGBA glowColour = COL_KNOB_GLOW) const
    {
        if (!name || name[0] == '\0')
            return 0.0f;
        const std::string namePart = std::string(name) + ":  ";
        const float textY = y + h / 2.0f;
        setFont(FONT_NAME_BOLD, fGeometry.labelFontSize);
        setFillColor(glowColour);
        cairo_save(fCR);
        // Clip to panel interior so long names cannot overflow
        cairo_rectangle(fCR, x + DISP_INSET, y + DISP_INSET,
                             w - DISP_INSET * 2.0f, h - DISP_INSET * 2.0f);
        cairo_clip(fCR);
        const float valueX = textLeft(x + DISP_TEXT_X, textY, namePart.c_str());
        if (value && value[0] != '\0')
            textLeft(valueX, textY, value);
        cairo_restore(fCR);
        return valueX;
    }

    // Draw value text only at a pre-computed valueX.
    // Clips to prevent overflow at the right panel edge.
    void drawParamValue(float x, float y, float w, float h,
                        float valueX, const char *value,
                        RGBA glowColour = COL_KNOB_GLOW) const
    {
        if (!value || value[0] == '\0')
            return;
        setFont(FONT_NAME_BOLD, fGeometry.labelFontSize);
        setFillColor(glowColour);
        cairo_save(fCR);
        cairo_rectangle(fCR, valueX, y + DISP_INSET,
                        (x + w - DISP_VALUE_RPAD) - valueX,
                        h - DISP_INSET * 2.0f);
        cairo_clip(fCR);
        textLeft(valueX, y + h / 2.0f, value);
        cairo_restore(fCR);
    }

    // Draw a button with lazy width measurement and caching.
    //
    // Left-anchor (normal) call:
    //   x  = left edge of button (not modified)
    //   w  = 0 on first call: drawButton measures and caches the width.
    //        On subsequent calls the cached value is reused.
    //
    // Right-anchor call:
    //   x  = right anchor on first call (when w < 0);  drawButton
    //        measures the label, sets x = x - w (left edge), then
    //        stores both back.  Subsequent calls have w > 0 and x
    //        already correct.
    //   w  = -1 on first call to signal right-anchor mode.
    void drawButton(float &x, float y, float &w, float h,
                    bool pressed, const char *label) const
    {
        // Font must be set first so that measurement is consistent
        const float btnFontSize =
            std::min(fGeometry.labelFontSize, h * BTN_LABEL_H_RATIO);
        setFont(FONT_NAME_REGULAR, btnFontSize);

        if (w == 0.0f) {
            // Left-anchor first call: measure and cache
            w = textMeasure(label) + PAGE_BTN_HPAD * 2.0f;
        } else if (w < 0.0f) {
            // Right-anchor first call: measure, compute left edge, store
            w = textMeasure(label) + PAGE_BTN_HPAD * 2.0f;
            x = x - w;
        }

        const float rr = h * BTN_CORNER_RATIO;
        const float ox = pressed ? BTN_PRESS_OFFSET : 0.0;
        const float oy = pressed ? BTN_PRESS_OFFSET : 0.0;

        // -- Shadow (unpressed only) -----------------------------------------
        if (!pressed) {
            cairo_save(fCR);
            cairo_translate(fCR, BTN_SHADOW_OX, BTN_SHADOW_OY);
            rrect(x, y, w, h, rr);
            cairo_set_source_rgba(fCR, 0, 0, 0, BTN_SHADOW_ALPHA);
            cairo_fill(fCR);
            cairo_restore(fCR);
        }

        // -- 3-stop gradient fill (top to bottom) ----------------------------
        rrect(x + ox, y + oy, w, h, rr);
        {
            cairo_pattern_t *p = cairo_pattern_create_linear(
                x + ox, y + oy, x + ox, y + oy + h);
            if (pressed) {
                cairo_pattern_add_color_stop_rgba(p, 0.0,
                                                  COL_BTN_PRS_TOP.r,
                                                  COL_BTN_PRS_TOP.g,
                                                  COL_BTN_PRS_TOP.b,
                                                  1.0);
                cairo_pattern_add_color_stop_rgba(p, BTN_GRAD_MID,
                                                  COL_BTN_PRS_MID.r,
                                                  COL_BTN_PRS_MID.g,
                                                  COL_BTN_PRS_MID.b,
                                                  1.0);
                cairo_pattern_add_color_stop_rgba(p, 1.0,
                                                  COL_BTN_PRS_BOT.r,
                                                  COL_BTN_PRS_BOT.g,
                                                  COL_BTN_PRS_BOT.b,
                                                  1.0);
            } else {
                cairo_pattern_add_color_stop_rgba(p, 0.0,
                                                  COL_BTN_OFF_TOP.r,
                                                  COL_BTN_OFF_TOP.g,
                                                  COL_BTN_OFF_TOP.b,
                                                  1.0);
                cairo_pattern_add_color_stop_rgba(p, BTN_GRAD_MID,
                                                  COL_BTN_OFF_MID.r,
                                                  COL_BTN_OFF_MID.g,
                                                  COL_BTN_OFF_MID.b,
                                                  1.0);
                cairo_pattern_add_color_stop_rgba(p, 1.0,
                                                  COL_BTN_OFF_BOT.r,
                                                  COL_BTN_OFF_BOT.g,
                                                  COL_BTN_OFF_BOT.b,
                                                  1.0);
            }
            cairo_set_source(fCR, p);
            cairo_fill(fCR);
            cairo_pattern_destroy(p);
        }

        // -- Border ----------------------------------------------------------
        rrect(x + ox, y + oy, w, h, rr);
        setStrokeColor(pressed ? COL_BTN_PRS_BORDER : COL_BTN_OFF_BORDER);
        cairo_set_line_width(fCR, BTN_BORDER_W);
        cairo_stroke(fCR);

        // -- Specular  -  top-half gradient clipped to button shape ----------
        cairo_save(fCR);
        rrect(x + ox, y + oy, w, h, rr);
        cairo_clip(fCR);
        {
            cairo_pattern_t *sp =
                cairo_pattern_create_linear(x + ox, y + oy,
                                            x + ox, y + oy + h * 0.5);
            const float sa = pressed ? COL_BTN_SPEC_PRS.a : COL_BTN_SPEC_OFF.a;
            cairo_pattern_add_color_stop_rgba(sp, 0.0, 1, 1, 1, sa);
            cairo_pattern_add_color_stop_rgba(sp, 1.0, 1, 1, 1, 0.0);
            cairo_set_source(fCR, sp);
            cairo_paint(fCR);
            cairo_pattern_destroy(sp);
        }
        cairo_restore(fCR);

        // -- Label  -  font already set at the top of this method ------------
        setFillColor(pressed ? COL_BTN_PRS_LABEL : COL_BTN_OFF_LABEL);
        textCentred(x + ox + w / 2.0f, y + oy + h / 2.0f, label);
    }

    // Rounded-rectangle path builder  -  public so that UIMenu and UILayout
    // can construct rrect paths before calling cairo_fill/cairo_stroke
    // directly on getCR().  Does not stroke or fill.
    void roundedRect(float x, float y,
                     float w, float h, float r) const
    {
        rrect(x, y, w, h, r);
    }

private:
    // -- Private helpers -----------------------------------------------------

    static RGBA lighten(const RGBA &c, float a)
    {
        auto cl = [](float v) { return v > 1.0f ? 1.0f : v; };
        return { cl(c.r + a), cl(c.g + a), cl(c.b + a), c.a };
    }

    static RGBA darken(const RGBA &c, float a)
    {
        auto cl = [](float v) { return v < 0.0f ? 0.0f : v; };
        return { cl(c.r - a), cl(c.g - a), cl(c.b - a), c.a };
    }

    // Rounded-rectangle path (does not stroke or fill).
    void rrect(float x, float y,
               float w, float h, float r) const
    {
        cairo_new_path(fCR);
        cairo_move_to(fCR, x + r, y);
        cairo_arc(fCR, x + w - r, y + r,     r, -M_PI / 2.0, 0);
        cairo_arc(fCR, x + w - r, y + h - r, r,  0,          M_PI / 2.0);
        cairo_arc(fCR, x + r,     y + h - r, r,  M_PI / 2.0, M_PI);
        cairo_arc(fCR, x + r,     y + r,     r,  M_PI,       3.0 * M_PI / 2.0);
        cairo_close_path(fCR);
    }

    // Set source to the pre-composited display background colour:
    //   rgba(0,0,0,DISP_FILL_ALPHA) composited over COL_BACKGROUND.
    void cairoSetDisplayBg() const
    {
        const float a  = DISP_FILL_ALPHA;
        const RGBA  &bg = COL_BACKGROUND;
        cairo_set_source_rgba(fCR, bg.r * (1.0 - a),
                                   bg.g * (1.0 - a),
                                   bg.b * (1.0 - a),
                                   1.0);
    }

    void drawTick(float cx, float cy, float tR,
                  float angle, bool isEnd, bool isActive) const
    {
        const float  tw = TRACK_W;
        float inner, outer, lw;
        RGBA  col;
        if (isEnd) {
            inner = tR - tw * TICK_END_INNER;
            outer = tR + tw * TICK_END_OUTER;
            lw    = TICK_END_WIDTH;
            col   = COL_TICK_END;
        } else if (isActive) {
            inner = tR - tw * TICK_STEP_INNER;
            outer = tR + tw * TICK_STEP_OUTER;
            lw    = TICK_ACTIVE_WIDTH;
            col   = COL_TICK_ACTIVE;
        } else {
            inner = tR - tw * TICK_STEP_INNER;
            outer = tR + tw * TICK_STEP_OUTER;
            lw    = TICK_STEP_WIDTH;
            col   = COL_TICK_STEP;
        }
        cairo_move_to(fCR, cx + std::cos(angle) * inner,
                           cy + std::sin(angle) * inner);
        cairo_line_to(fCR, cx + std::cos(angle) * outer,
                           cy + std::sin(angle) * outer);
        setStrokeColor(col);
        cairo_set_line_width(fCR, lw);
        cairo_set_line_cap(fCR, CAIRO_LINE_CAP_ROUND);
        cairo_stroke(fCR);
    }

    // Initialise FreeType and load both font faces from the embedded
    // header arrays.  Called once from the constructor.
    void initFonts()
    {
        if (FT_Init_FreeType(&fFTLib) != 0) {
            fFTLib = nullptr;
            return;
        }
        if (FT_New_Memory_Face(fFTLib,
                reinterpret_cast<const FT_Byte *>(NotoSans_Regular_ttf),
                static_cast<FT_Long>(sizeof(NotoSans_Regular_ttf)),
                0, &fFTRegular) != 0)
            fFTRegular = nullptr;

        if (FT_New_Memory_Face(fFTLib,
                reinterpret_cast<const FT_Byte *>( NotoSans_Bold_ttf),
                static_cast<FT_Long>(sizeof(NotoSans_Bold_ttf)),
                0, &fFTBold) != 0)
            fFTBold = nullptr;

        if (fFTRegular)
            fFaceReg  = cairo_ft_font_face_create_for_ft_face( fFTRegular, 0);
        if (fFTBold)
            fFaceBold = cairo_ft_font_face_create_for_ft_face( fFTBold, 0);
    }
};
