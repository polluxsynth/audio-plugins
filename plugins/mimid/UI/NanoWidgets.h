/*
 * NanoWidgets.h  -  NanoVG drawing functions for MiMi-d UI
 *
 * Widget library for MiMi-d using NanoVG (GPU accelerated). This offers
 * the advantage of fast CPU accelerated rendering, especially important
 * when updating parameter values graphically.
 *   - Call setVG(vg) at the start of each frame before drawing
 *   - getVG() exposes the NanoVG reference for callers that need it
 *     directly (UIMenu, UILayout)
 *   - Fonts are looked up by name ("regular", "bold"); load them with
 *     those names via createFontFromFile() in loadSharedResources()
 *   - Knob face uses a 2-stop radial gradient.
 *   - NanoVG renders directly to the OpenGL framebuffer every frame
 *
 * Build: DISTRHO_UI_USE_NANOVG=1
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
#include "NanoVG.hpp"
#include <cmath>
#include <string>
#include <cstdio>
#include "UIConstants.h"
#include "UIGeometry.h"
#include "UISettings.h"

// ---------------------------------------------------------------------------
// Color conversion helper  -  used by NanoWidgets and callers using getVG()
// ---------------------------------------------------------------------------

static inline Color toColor(const RGBA &c)
{
    return Color(c.r, c.g, c.b, c.a);
}

// ---------------------------------------------------------------------------
// NanoWidgets
// ---------------------------------------------------------------------------

class NanoWidgets {
    NanoVG    *fVG = nullptr;
    UIGeometry fGeometry;
    UISettings fSettings;

public:
    NanoWidgets() = default;

    NanoWidgets(const UIGeometry &g, const UISettings &s)
        : fGeometry(g), fSettings(s) {}

    // -- Context --------------------------------------------------------------

    // Call at the start of each frame before any drawing.
    void setVG(NanoVG &vg)  { fVG = &vg; }

    // Expose context for callers needing it directly (UIMenu, UILayout).
    NanoVG &getVG() const   { return *fVG; }

    void updateSettings(const UISettings &s) { fSettings = s; }

    const UIGeometry &geometry() const { return fGeometry; }
    const UISettings &settings() const { return fSettings; }

    // -- Low-level helpers (also used by UIMenu/UILayout) ---------------------

    void setFillColor(const RGBA &c) const
    {
        fVG->fillColor(c.r, c.g, c.b, c.a);
    }

    void setStrokeColor(const RGBA &c) const
    {
        fVG->strokeColor(c.r, c.g, c.b, c.a);
    }

    // Horizontally and vertically centred text at (cx, cy).
    float textCentred(float cx, float cy, const char *txt) const
    {
        fVG->textAlign(NanoVG::ALIGN_CENTER | NanoVG::ALIGN_MIDDLE);
        return fVG->text(cx, cy, txt, nullptr);
    }

    // Left-aligned, vertically centred text at (x, y).
    float textLeft(float x, float y, const char *txt) const
    {
        fVG->textAlign(NanoVG::ALIGN_LEFT | NanoVG::ALIGN_MIDDLE);
        return fVG->text(x, y, txt, nullptr);
    }

    // Horizontally centred, top-aligned text at (cx, y).
    float textTop(float cx, float y, const char *txt) const
    {
        fVG->textAlign(NanoVG::ALIGN_CENTER | NanoVG::ALIGN_TOP);
        return fVG->text(cx, y, txt, nullptr);
    }

    // Measure text width without drawing. Font/face must be set beforehand.
    float textMeasure(const char *txt) const
    {
        Rectangle<float> tb;
        fVG->textBounds(0, 0, txt, nullptr, tb);
        return tb.getWidth();
    }

    // -- Public drawing interface ---------------------------------------------

    // -- Symbols --------------------------------------------------------------

    // Draw one symbol of type SYM_* centred at (cx, cy), scaled by s
    // (= SYM_SIZE).  Waveform geometry is governed by SYM_HW (half-width),
    // SYM_HH (half-height), and SYM_PW (pulse width fraction), all in
    // UIConstants.h.
    void drawSymbol(Symbol symbol, float cx, float cy, float s) const
    {
        if (symbol == SYM_NONE) return;
        const float hw  = s * SYM_HW;
        const float hh  = s * SYM_HH;
        const float x0  = cx - hw, x1 = cx + hw;
        const float y0  = cy - hh, y1 = cy + hh;
        const float mx  = cx;
        const float my  = cy;
        const float pw  = s * SYM_PW;

        fVG->beginPath();
        switch (symbol) {
            case SYM_SAW:
                fVG->moveTo(x0, y1);
                fVG->lineTo(x1, y0);
                fVG->lineTo(x1, y1);
                break;
            case SYM_RSAW:
                fVG->moveTo(x0, y1);
                fVG->lineTo(x0, y0);
                fVG->lineTo(x1, y1);
                break;
            case SYM_TRI:
                fVG->moveTo(x0, y1);
                fVG->lineTo(mx, y0);
                fVG->lineTo(x1, y1);
                break;
            case SYM_SQUARE:
                fVG->moveTo(x0, my);
                fVG->lineTo(x0, y0);
                fVG->lineTo(mx, y0);
                fVG->lineTo(mx, y1);
                fVG->lineTo(x1, y1);
                fVG->lineTo(x1, my);
                break;
            case SYM_PULSE:
                fVG->moveTo(x0,      y1);
                fVG->lineTo(x0,      y0);
                fVG->lineTo(x0 + pw, y0);
                fVG->lineTo(x0 + pw, y1);
                fVG->lineTo(x1,      y1);
                break;
            case SYM_RPULSE:
                fVG->moveTo(x0,      y0);
                fVG->lineTo(x1 - pw, y0);
                fVG->lineTo(x1 - pw, y1);
                fVG->lineTo(x1,      y1);
                fVG->lineTo(x1,      y0);
                break;
            default: return;
        }
        fVG->strokeWidth(SYM_LW);
        fVG->lineCap(NanoVG::SQUARE);
        fVG->lineJoin(NanoVG::MITER);
        fVG->stroke();
    }

    // Draw all symbols for a knob.  Each entry is a {angleDeg, symbol} pair
    // where angleDeg is degrees from the knob start point (0 = bottom-left,
    // 270 = bottom-right).  Symbols are placed outside the track + ticks so
    // they are unaffected by partial knob redraws.
    // symCentreR places the symbol centre such that the inner edge of the
    // symbol stroke (hh inside the centre) begins at trackOuter + SYM_DIST,
    // giving a consistent gap at all angles.
    void drawSymbols(
        float cx, float cy,
        const std::vector<std::pair<float,Symbol>> &points) const
    {
        if (points.empty()) return;
        const float trackOuter = fGeometry.trackR
                               + fGeometry.tw * TICK_STEP_OUTER
                               + TICK_STEP_WIDTH * 0.5f;
        const float hh         = SYM_SIZE * SYM_HH;
        const float symCentreR = trackOuter + SYM_DIST + hh;
        const float startRad =
            ARC_START_DEG * static_cast<float>(M_PI) / 180.0f;

        cy += SYM_Y_NUDGE;
        setStrokeColor(COL_KNOB_LABEL);
        for (const auto &pt : points) {
            const float angle = startRad
                + pt.first * static_cast<float>(M_PI) / 180.0f;
            const float sx = cx + std::cos(angle) * symCentreR;
            const float sy = cy + std::sin(angle) * symCentreR;
            drawSymbol(pt.second, sx, sy, SYM_SIZE);
        }
    }

    void drawBackground(float w, float h) const
    {
        fVG->beginPath();
        fVG->rect(0, 0, w, h);
        setFillColor(COL_BACKGROUND);
        fVG->fill();
    }

    // Clear the bounding box of a knob to the background colour before a
    // partial redraw.  The circle radius covers the track arc, tick marks,
    // shadow, and glow ring.  A circle fits the round knob more closely
    // than a rectangle, leaving less of the background needlessly repainted.
    // A scissor clamps the bottom edge to the same position as the original
    // rectangle bottom (cy + trackR + tw/2), protecting the name label below.
    void clearKnobArea(float cx, float cy) const
    {
        const float tickReach = fGeometry.trackR
                              + fGeometry.tw * TICK_STEP_OUTER
                              + TICK_STEP_WIDTH * 0.25f;
        const float circleTop  = cy - tickReach;
        const float clipBottom = cy + fGeometry.trackR
                               + fGeometry.tw * 0.5f;
        fVG->scissor(cx - tickReach, circleTop,
                     tickReach * 2.0f, clipBottom - circleTop);
        fVG->beginPath();
        fVG->circle(cx, cy, tickReach);
        setFillColor(COL_BACKGROUND);
        fVG->fill();
        fVG->resetScissor();
    }

    // showIndex, showArc, showValue come from fSettings.
    // glowColour is caller-supplied as it varies by page.
    void drawKnob(float cx, float cy,
                  float norm, int steps, bool isInteger,
                  const char *label, const char *valueTxt,
                  float zeroNorm    = -1.0f,
                  bool selected     = false,
                  bool drawName     = true,
                  RGBA glowColour   = COL_KNOB_GLOW) const
    {
        const UIGeometry &g = fGeometry;
        const float r     = g.r;
        const float tR    = g.trackR;
        const float tw    = g.tw;
        const float fcx   = cx;
        const float fcy   = cy;
        const float angle = g.valueToAngle(norm);

        // Shadow  -  vertically squashed ellipse offset below/right
        // Use save/restore when squashing a circle into an ellipse.
        if (KNOB_SHADOW_BLUR > 0.0f) {
            fVG->save();
            fVG->translate(fcx + KNOB_SHADOW_BLUR * KNOB_SHADOW_OFFSET_X,
                           fcy + KNOB_SHADOW_BLUR * KNOB_SHADOW_OFFSET_Y);
            fVG->scale(1.0f, KNOB_SHADOW_SQUASH);
            fVG->beginPath();
            fVG->circle(0, 0, r + KNOB_SHADOW_RADIUS);
            fVG->fillColor(0.0f, 0.0f, 0.0f, KNOB_SHADOW_ALPHA);
            fVG->fill();
            fVG->restore();
        }

        // Track background
        if (fSettings.showArc) {
            fVG->beginPath();
            fVG->arc(fcx, fcy, tR,
                     g.arcStart, g.arcStart + g.arcRange,
                     NanoVG::CW);
            setStrokeColor(COL_TRACK_BG);
            fVG->strokeWidth(tw);
            fVG->lineCap(NanoVG::ROUND);
            fVG->stroke();
        }

        // Track fill
        if (fSettings.showArc) {
            if (zeroNorm >= 0.0f) {
                const float zeroAngle = g.valueToAngle(zeroNorm);
                if (norm > zeroNorm) {
                    fVG->beginPath();
                    fVG->arc(fcx, fcy, tR, zeroAngle, angle, NanoVG::CW);
                    setStrokeColor(COL_TRACK_FILL);
                    fVG->strokeWidth(tw);
                    fVG->lineCap(NanoVG::ROUND);
                    fVG->stroke();
                } else if (norm < zeroNorm) {
                    fVG->beginPath();
                    fVG->arc(fcx, fcy, tR, zeroAngle, angle, NanoVG::CCW);
                    setStrokeColor(COL_TRACK_FILL);
                    fVG->strokeWidth(tw);
                    fVG->lineCap(NanoVG::ROUND);
                    fVG->stroke();
                }
            } else {
                // 0.002 threshold avoids drawing a tiny arc stub at
                // minimum value
                if (norm > 0.002f) {
                    fVG->beginPath();
                    fVG->arc(fcx, fcy, tR, g.arcStart, angle, NanoVG::CW);
                    setStrokeColor(COL_TRACK_FILL);
                    fVG->strokeWidth(tw);
                    fVG->lineCap(NanoVG::ROUND);
                    fVG->stroke();
                }
            }
        }

        // Bipolar zero tick  -  always drawn
        if (zeroNorm >= 0.0f)
            drawTick(cx, cy, tR, g.valueToAngle(zeroNorm), true, false);

        // End ticks  -  always drawn
        drawTick(cx, cy, tR, g.arcStart,              true, false);
        drawTick(cx, cy, tR, g.arcStart + g.arcRange, true, false);

        // Step ticks
        if (steps > 1) {
            for (int i = 0; i <= steps; i++)
                // Set last parameter to  i == (int)std::round(norm * steps)
                // for highlighting the active step.
                drawTick(cx, cy, tR, g.valueToAngle((float)i / steps), false, false);
        }

        // Rim stroke
        fVG->beginPath();
        fVG->circle(fcx, fcy, r);
        setStrokeColor(COL_KNOB_RIM);
        fVG->strokeWidth(KNOB_RIM_W);
        fVG->stroke();

        // Face  -  radial gradient: flat bright centre (0..KNOB_FLAT_ZONE) then
        // darkening to rim. Inner radius set to the flat zone boundary so the
        // gradient only applies in the outer ring.
        {
            const RGBA base = COL_KNOB_BASE;
            const RGBA cen  = lighten(base, KNOB_CENTRE_LIFT);
            const RGBA rim  = darken(base, KNOB_RIM_DARKEN);
            const float flatR = r * KNOB_FLAT_ZONE;
            const NanoVG::Paint face = fVG->radialGradient(
                fcx, fcy, flatR, r, toColor(cen), toColor(rim));
            fVG->beginPath();
            fVG->circle(fcx, fcy, r);
            fVG->fillPaint(face);
            fVG->fill();
        }

        // Specular  -  top-edge linear gradient fading downward
        {
            const NanoVG::Paint spec = fVG->linearGradient(
                fcx, fcy - r,
                fcx, fcy - r * KNOB_SPEC_FADE_Y,
                Color(1.0f, 1.0f, 1.0f, KNOB_SPEC_OPACITY),
                Color(1.0f, 1.0f, 1.0f, 0.0f));
            fVG->beginPath();
            fVG->circle(fcx, fcy, r * KNOB_SPEC_RADIUS);
            fVG->fillPaint(spec);
            fVG->fill();
        }

        // Indicator line
        if (fSettings.showIndex) {
            const float i0 = r * IND_R_INNER;
            const float i1 = r * IND_R_OUTER;
            const float lw = std::max(IND_WIDTH_MIN, r * IND_WIDTH_RATIO);
            fVG->beginPath();
            fVG->moveTo(fcx + cosf(angle) * i0, fcy + sinf(angle) * i0);
            fVG->lineTo(fcx + cosf(angle) * i1, fcy + sinf(angle) * i1);
            setStrokeColor(COL_INDICATOR);
            fVG->strokeWidth(lw);
            fVG->lineCap(NanoVG::ROUND);
            fVG->stroke();
        }

        // Selection glow  -  narrow stroked ring just outside the knob edge,
        // with a radial gradient paint so it fades toward the arc track.
        if (selected) {
            const float glowOuter = tR + tw * 0.5f;
            const float glowInner = r + KNOB_RIM_W * 0.5f;
            const float ringW     = (glowOuter - glowInner) * KNOB_GLOW_EXTENT;
            const float ringR     = glowInner + ringW * 0.5f;
            const NanoVG::Paint glow = fVG->radialGradient(
                fcx, fcy, glowInner, glowOuter,
                Color(glowColour.r, glowColour.g, glowColour.b,
                      KNOB_GLOW_PEAK_ALPHA),
                Color(glowColour.r, glowColour.g, glowColour.b, 0.0f));
            fVG->beginPath();
            fVG->circle(fcx, fcy, ringR);
            fVG->strokePaint(glow);
            fVG->strokeWidth(ringW);
            fVG->stroke();
        }

        // Value text (centred on knob face)
        const bool drawValueText = (fSettings.showValue == SHOW_ALL) ||
                                   (fSettings.showValue == SHOW_STEPPED &&
                                    (steps > 0 || isInteger));
        if (drawValueText) {
            fVG->fontSize(g.fontSize);
            fVG->fontFace(FONT_NAME_BOLD);
            setFillColor(COL_VALUE_TEXT);
            textCentred(fcx, fcy, valueTxt);
        }

        // Name label (below knob)
        if (drawName) {
            fVG->fontSize(g.labelFontSize);
            fVG->fontFace(FONT_NAME_REGULAR);
            setFillColor(COL_NAME_TEXT);
            textTop(fcx, cy + g.nameDY, label);
        }
    }

    void drawGroupFrame(float fx, float fy, float fw, float fh,
                        const char *title) const
    {
        const UIGeometry &g = fGeometry;
        const float rr   = FRAME_CORNER_R;
        const float lx   = fx + FRAME_TITLE_X;
        const float lpad = FRAME_TITLE_PAD;

        // Title text first, vertically centred on the top frame line.
        // nvgText() returns the x position after the last glyph; use
        // that to derive the gap width so no separate measurement is
        // needed.
        fVG->fontSize(g.labelFontSize);
        fVG->fontFace(FONT_NAME_BOLD);
        setFillColor(COL_FRAME_TITLE);
        const float textEndX = textLeft(lx, fy, title);
        const float gapW = (textEndX - lx) + lpad * 2.0f;

        // Frame path with gap at top-left for the title text.
        // Drawn after the text so the line does not overdraw it.
        fVG->beginPath();
        fVG->moveTo(lx + gapW, fy);
        fVG->lineTo(fx + fw - rr, fy);
        fVG->arcTo(fx + fw, fy,
                   fx + fw, fy + rr, rr);
        fVG->lineTo(fx + fw, fy + fh - rr);
        fVG->arcTo(fx + fw, fy + fh,
                   fx + fw - rr, fy + fh, rr);
        fVG->lineTo(fx + rr, fy + fh);
        fVG->arcTo(fx, fy + fh,
                   fx, fy + fh - rr, rr);
        fVG->lineTo(fx, fy + rr);
        fVG->arcTo(fx, fy,
                   fx + rr, fy, rr);
        fVG->lineTo(lx - lpad, fy);
        setStrokeColor(COL_FRAME_LINE);
        fVG->strokeWidth(FRAME_LINE_W);
        fVG->stroke();
    }

    // -- Parameter display panel ------------------------------------------
    //
    //   clearDisplayArea        -  reset panel area to display background
    //   clearDisplayValueArea   -  reset value field only (keeps name)
    //   drawParamDisplayFrame   -  frame: border, inner shadow.
    //   drawParamText           -  draw name + value; returns valueX (left edge
    //                              of value field) for the caller to cache.
    //   drawParamValue          -  draw value at a pre-computed valueX.
    //
    // For partial redraws (value only), call clearDisplayValueArea which
    // just clears the value area in preparation for drawing the new value,
    // It assumes that clearDisplayArea has been called at some earlier point
    // in time, as it actually clears a slightly smaller area in order to
    // use a straight rect call since the left edge (towards the name area)
    // does not need a rounded rect.

    void clearDisplayArea(float x, float y, float w, float h) const
    {
        fVG->beginPath();
        fVG->roundedRect(x, y, w, h, DISP_CORNER_R);
        setFillColor(COL_DISPLAY_BG);
        fVG->fill();
    }

    void drawParamDisplayFrame(float x, float y, float w, float h) const
    {
        // Border
        fVG->beginPath();
        fVG->roundedRect(x + 0.5f, y + 0.5f,
                         w - DISP_BORDER_STROKE_W, h - DISP_BORDER_STROKE_W,
                         DISP_CORNER_R - 0.5f);
        fVG->strokeColor(0.0f, 0.0f, 0.0f, DISP_BORDER_A);
        fVG->strokeWidth(DISP_BORDER_STROKE_W);
        fVG->stroke();

        // Inner shadow  -  dark line just inside the top edge
        fVG->beginPath();
        fVG->moveTo(x + DISP_CORNER_R,
                    y + DISP_SHADOW_Y);
        fVG->lineTo(x + w - DISP_CORNER_R,
                    y + DISP_SHADOW_Y);
        fVG->strokeColor(0.0f, 0.0f, 0.0f, DISP_SHADOW_A);
        fVG->strokeWidth(DISP_SHADOW_STROKE_W);
        fVG->stroke();
    }

    // Used for partial redraws. Just clear the value area, observing
    // a double stroke width to avoid any collision with the frame.
    void clearDisplayValueArea(float x, float y, float w, float h) const
    {
        const float inset = DISP_BORDER_STROKE_W * 2.0f;
        const float rx = x;
        const float ry = y + inset;
        const float rw = w - inset;
        const float rh = h - inset * 2.0f;
        fVG->beginPath();
        fVG->rect(rx, ry, rw, rh);
        // clear
        setFillColor(COL_DISPLAY_BG);
        fVG->fill();
    }

    // Draw name + value in the display panel.  Draws the name prefix first,
    // measures its advance to locate the value field, then draws the value.
    // Returns valueX (left edge of value field) so the caller can cache it
    // for subsequent drawParamValue calls without re-measuring.
    float drawParamText(float x, float y, float h,
                        const char *name, const char *value,
                        RGBA glowColour = COL_KNOB_GLOW) const
    {
        if (!name || name[0] == '\0') return 0.0f;
        const std::string namePart = std::string(name) + ":  ";
        const float textY = y + h / 2.0f;
        setFillColor(glowColour);
        fVG->fontSize(fGeometry.labelFontSize);
        fVG->fontFace(FONT_NAME_BOLD);
        // textLeft() returns the x position after the last glyph —
        // that is valueX.
        const float valueX = textLeft(x + DISP_TEXT_X, textY, namePart.c_str());
        if (value && value[0] != '\0')
            textLeft(valueX, textY, value);
        return valueX;
    }

    // Draw value text at a pre-computed valueX.
    void drawParamValue(float y, float h,
                        float valueX, const char *value,
                        RGBA glowColour = COL_KNOB_GLOW) const
    {
        if (!value || value[0] == '\0') return;
        setFillColor(glowColour);
        fVG->fontSize(fGeometry.labelFontSize);
        fVG->fontFace(FONT_NAME_BOLD);
        textLeft(valueX, y + h / 2.0f, value);
    }

    // Draw a button, with lazy width measurement and caching.
    //
    // Left-anchor (normal) call:
    //   x  = left edge of button  (not modified)
    //   w  = 0 on first call: drawButton measures and caches the label width.
    //        Subsequent calls reuse the cached w.
    //
    // Right-anchor call:
    //   x  = right anchor on first call (w < 0); drawButton measures the label,
    //        computes x = x - w (left edge), and stores both back
    //        via references.
    //        Subsequent calls have w > 0 and x already set, so just draw.
    //   w  = -1 on first call to signal right-anchor mode.
    void drawButton(float &x, float y, float &w, float h,
                    bool pressed, const char *label) const
    {
        // Font set first — needed for width measurement
        const float btnFontSize = std::min(fGeometry.labelFontSize,
                                           h * BTN_LABEL_H_RATIO);
        fVG->fontSize(btnFontSize);
        fVG->fontFace(FONT_NAME_REGULAR);

        if (w == 0.0f) {
            // Left-anchor first call: measure and cache
            w = textMeasure(label) + PAGE_BTN_HPAD * 2.0f;
        } else if (w < 0.0f) {
            // Right-anchor first call: measure, compute and store left edge
            w = textMeasure(label) + PAGE_BTN_HPAD * 2.0f;
            x = x - w;
        }

        const float rr = h * BTN_CORNER_RATIO;
        const float ox = pressed ? BTN_PRESS_OFFSET : 0.0f;
        const float oy = pressed ? BTN_PRESS_OFFSET : 0.0f;

        // Shadow (unpressed only)
        if (!pressed) {
            fVG->beginPath();
            fVG->roundedRect(x + BTN_SHADOW_OX,
                             y + BTN_SHADOW_OY,
                             w, h, rr);
            fVG->fillColor(0.0f, 0.0f, 0.0f, BTN_SHADOW_ALPHA);
            fVG->fill();
        }

        // Gradient fill (top-to-bottom, 2-stop)
        {
            const Color top = pressed ? toColor(COL_BTN_PRS_TOP)
                                      : toColor(COL_BTN_OFF_TOP);
            const Color bot = pressed ? toColor(COL_BTN_PRS_BOT)
                                      : toColor(COL_BTN_OFF_BOT);
            const NanoVG::Paint grad = fVG->linearGradient(
                x + ox, y + oy,
                x + ox, y + oy + h,
                top, bot);
            fVG->beginPath();
            fVG->roundedRect(x + ox, y + oy, w, h, rr);
            fVG->fillPaint(grad);
            fVG->fill();
        }

        // Border
        fVG->beginPath();
        fVG->roundedRect(x + ox, y + oy, w, h, rr);
        setStrokeColor(pressed ? COL_BTN_PRS_BORDER : COL_BTN_OFF_BORDER);
        fVG->strokeWidth(BTN_BORDER_W);
        fVG->stroke();

        // Specular  -  top-half linear gradient fading to transparent
        {
            const float sa = pressed ? COL_BTN_SPEC_PRS.a
                                     : COL_BTN_SPEC_OFF.a;
            const NanoVG::Paint spec = fVG->linearGradient(
                x + ox, y + oy,
                x + ox, y + oy + h * 0.5f,
                Color(1.0f, 1.0f, 1.0f, sa),
                Color(1.0f, 1.0f, 1.0f, 0.0f));
            fVG->beginPath();
            fVG->roundedRect(x + ox, y + oy, w, h, rr);
            fVG->fillPaint(spec);
            fVG->fill();
        }

        // Label  -  font already set above
        setFillColor(pressed ? COL_BTN_PRS_LABEL : COL_BTN_OFF_LABEL);
        textCentred(x + ox + w / 2.0f, y + oy + h / 2.0f, label);
    }

private:
    // -- Helpers --------------------------------------------------------------

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

    void drawTick(float cx, float cy, float tR,
                  float angle, bool isEnd, bool isActive) const
    {
        float tw = TRACK_W, inner, outer, lw;
        RGBA col;
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
        fVG->beginPath();
        fVG->moveTo(cx + cosf(angle) * inner, cy + sinf(angle) * inner);
        fVG->lineTo(cx + cosf(angle) * outer, cy + sinf(angle) * outer);
        setStrokeColor(col);
        fVG->strokeWidth(lw);
        fVG->lineCap(NanoVG::ROUND);
        fVG->stroke();
    }
};
