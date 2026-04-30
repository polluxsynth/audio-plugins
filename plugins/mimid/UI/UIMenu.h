/*
 * UIMenu.h  -  UI menu and splash screen for MiMi-d UI
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

#include <cmath>
#include <functional>
#include "UIConstants.h"
#include "UISettings.h"
#include "NanoWidgets.h"
#include "UILayoutDefs.h"

namespace {
enum MenuItemId {
    MENU_NONE = -1,
    MENU_UI_SCALE = 0, MENU_RESTART_NOTE,
    MENU_SHOW_INDEX, MENU_SHOW_ARC, MENU_SHOW_VALUE,
    MENU_SNAP_SCALE,
    MENU_GHOST_MODE, MENU_ABOUT, MENU_ITEM_COUNT
};
enum MenuItemType {
    MIT_HEADER, MIT_NUMERIC, MIT_BOOL, MIT_TRISTATE, MIT_INFO, MIT_ACTION,
    MIT_SEP
};
struct MenuItemDef {
    MenuItemId   id;
    MenuItemType type;
    const char  *label;
};
static const MenuItemDef kMenuItems[] = {
    { MENU_NONE,         MIT_HEADER,   "UI Settings"               },
    { MENU_NONE,         MIT_SEP,      nullptr                     },
    { MENU_UI_SCALE,     MIT_NUMERIC,  "UI scale"                  },
    { MENU_RESTART_NOTE, MIT_INFO,     "(Restart to take effect)"  },
    { MENU_NONE,         MIT_SEP,      nullptr                     },
    { MENU_SHOW_INDEX,   MIT_BOOL,     "Knob index"                },
    { MENU_SHOW_ARC,     MIT_BOOL,     "Knob arc"                  },
    { MENU_SHOW_VALUE,   MIT_TRISTATE, "Knob value"                },
    { MENU_NONE,         MIT_SEP,      nullptr                     },
    { MENU_SNAP_SCALE,   MIT_BOOL,     "Snap scale"                },
    { MENU_NONE,         MIT_SEP,      nullptr                     },
    { MENU_GHOST_MODE,   MIT_BOOL,     "Ghost mode"                },
    { MENU_NONE,         MIT_SEP,      nullptr                     },
    { MENU_ABOUT,        MIT_ACTION,   "About"                     },
};
static const int kMenuItemCount = (int)(sizeof(kMenuItems)/sizeof(kMenuItems[0]));
} // anonymous namespace

// MenuGeometry is outside the anonymous namespace so UIMenu can use
// it as a member without triggering -Wsubobject-linkage.
struct MenuGeometry {
    float w, h;
    float itemY[kMenuItemCount];
    float itemH[kMenuItemCount];
};

class UIMenu {
    UISettings                              &fSettings;
    std::function<void(const UISettings &)>  fOnSettingsChanged;
    bool         fMenuOpen        = false;
    bool         fSplashOpen      = false;
    bool         fNeedsFullRedraw = false;
    float        fMenuX           = 0.0f;
    float        fMenuY           = 0.0f;
    int          fHover           = -1;
    int          fDragRow         = -1;
    float        fDragStartY      = 0.0f;
    float        fDragStartVal    = 0.0f;
    // row currently armed for editing, -1 = none
    int          fArmed           = -1;
    // value saved at arm time, restored on Esc
    float        fArmedOrigVal    = 0.0f;
    MenuGeometry fGeom            = {};
    std::string  fVersion         = {};

public:
    UIMenu(UISettings &settings,
           std::function<void(const UISettings &)> onSettingsChanged,
           std::string version)
        : fSettings(settings), fOnSettingsChanged(onSettingsChanged),
          fVersion(std::move(version))
    {
        // All geometry depends only on constants  -  compute once at
        // construction.
        float cy = MENU_PAD_Y;
        for (int i = 0; i < kMenuItemCount; i++) {
            fGeom.itemY[i] = cy;
            float h = (kMenuItems[i].type == MIT_SEP) ? MENU_SEP_H : MENU_ITEM_H;
            fGeom.itemH[i] = h;
            cy += h;
        }
        fGeom.h = cy + MENU_PAD_Y;
        fGeom.w = MENU_MIN_W;
    }

    bool isOpen()       const { return fMenuOpen;   }
    bool isSplashOpen() const { return fSplashOpen; }

    bool takeNeedsFullRedraw()
    {
        bool v = fNeedsFullRedraw;
        fNeedsFullRedraw = false;
        return v;
    }

    void open(float mx, float my, float canvasW, float canvasH)
    {
        float x = mx, y = my;
        if (x + fGeom.w > canvasW - MENU_EDGE_MARGIN)
            x = canvasW - fGeom.w - MENU_EDGE_MARGIN;
        if (y + fGeom.h > canvasH - MENU_EDGE_MARGIN)
            y = canvasH - fGeom.h - MENU_EDGE_MARGIN;
        if (x < MENU_EDGE_MARGIN) x = MENU_EDGE_MARGIN;
        if (y < MENU_EDGE_MARGIN) y = MENU_EDGE_MARGIN;
        fMenuX = x;
        fMenuY = y;
        fHover = -1;
        fDragRow = -1;
        fMenuOpen = true;
        fNeedsFullRedraw = true;
    }

    void close()
    {
        if (fArmed >= 0) {
            // restore in-memory only
            applyNumeric(kMenuItems[fArmed].id, fArmedOrigVal, false);
            fArmed = -1;
        }
        fMenuOpen = false;
        fDragRow = -1;
        fHover = -1;
        fNeedsFullRedraw = true;
    }
    void closeSplash() { fSplashOpen = false; fNeedsFullRedraw = true; }

    void draw(const NanoWidgets &wc, float canvasW, float canvasH,
              float titleFontSize, float scale)
    {
        if (!fSplashOpen && !fMenuOpen) return;
        NanoVG &vg = wc.getVG();
        vg.save();
        vg.scale(scale, scale);
        if (fSplashOpen)
            // Splash uses fixed coordinates and thus doesn't need to
            // make any internal considerations regarding scale.
            drawSplash(wc, canvasW, canvasH, titleFontSize);
        else
            // Meny uses mouse coordinates as origin, so needs to know scale
            // on order to snap starting coordinate to physical pixels when
            // resizing down to avoid fuzzyness.
            drawMenu(wc, scale);
        vg.restore();
    }

    bool onMouse(float mx, float my, bool press, int button)
    {
        if (fSplashOpen) {
            if (press) closeSplash();
            return true;
        }
        if (!fMenuOpen)  return false;
        if (button != 1) return true;
        if (!press) {
            fDragRow = -1;
            return true;
        }
        int row = hitTest(mx, my);
        if (row < 0) {
            close();
            return true;
        }
        if (kMenuItems[row].type == MIT_NUMERIC) {
            if (fArmed == row) {
                // Second click on armed item  -  confirm
                confirmArmed();
            } else {
                // First click  -  confirm any previously armed item,
                // arm this one
                confirmArmed();
                fArmed = row;
                fArmedOrigVal = // (kMenuItems[row].id == MENU_UI_SCALE) ?
                                fSettings.uiScale;
                fDragRow = row;
                fDragStartY = my;
                fDragStartVal = fArmedOrigVal;
            }
            return true;
        }
        confirmArmed(); // confirm any armed item before activating another
        activate(row);
        return true;
    }

    bool onMotion(float mx, float my)
    {
        if (fSplashOpen) return true;
        if (!fMenuOpen)  return false;
        if (fDragRow >= 0) {
            float delta = (fDragStartY - my) / MENU_DRAG_RANGE * MENU_DRAG_UNITS;
            float newVal = fDragStartVal + std::round(delta / MENU_STEP) * MENU_STEP;
            applyNumeric(kMenuItems[fDragRow].id, newVal);
            fNeedsFullRedraw = true;
            return true;
        }
        int row = hitTest(mx, my);
        if (row != fHover) {
            fHover = row;
            fNeedsFullRedraw = true;
        }
        return true;
    }

    bool onScroll(float delta)
    {
        if (fSplashOpen) return true;
        if (!fMenuOpen)  return false;
        if (fArmed >= 0 && kMenuItems[fArmed].type == MIT_NUMERIC)
            adjustNumeric(fArmed, delta);
        return true;
    }

    bool onKey(int key, bool press,
               int kKeyUp, int kKeyDown, int kKeyLeft, int kKeyRight,
               int kKeyPageUp, int kKeyPageDown)
    {
        if (fSplashOpen) {
            if (press) closeSplash();
            return true;
        }
        if (!fMenuOpen)  return false;
        if (!press) return true;
        if (key == 27) { // Escape
            if (fArmed >= 0) {
                // Esc while armed  -  restore original value and
                // disarm without notifying
                applyNumeric(kMenuItems[fArmed].id, fArmedOrigVal, false);
                fArmed = -1;
                fNeedsFullRedraw = true;
            } else {
                close();
            }
            return true;
        }
        if (key == kKeyUp || key == kKeyDown) {
            confirmArmed(); // confirm before navigating away
            int dir = (key == kKeyDown) ? 1 : -1;
            int next = fHover + dir;
            for (int t = 0; t < kMenuItemCount; t++) {
                if (next < 0) next = kMenuItemCount - 1;
                if (next >= kMenuItemCount) next = 0;
                MenuItemType mt = kMenuItems[next].type;
                if (mt == MIT_NUMERIC || mt == MIT_BOOL ||
                    mt == MIT_TRISTATE || mt == MIT_ACTION) break;
                next += dir;
            }
            fHover = next;
            fNeedsFullRedraw = true;
            return true;
        }
        if (key == kKeyLeft || key == kKeyRight) {
            if (fArmed >= 0 && kMenuItems[fArmed].type == MIT_NUMERIC)
                adjustNumeric(fArmed, key == kKeyRight ? 1.0f : -1.0f);
            return true;
        }
        if (key == kKeyPageUp || key == kKeyPageDown) {
            if (fArmed >= 0 && kMenuItems[fArmed].type == MIT_NUMERIC)
                adjustNumeric(fArmed, key == kKeyPageUp ? 1.0f : -1.0f);
            return true;
        }
        if (key == '\n' || key == '\r' || key == ' ') {
            if (fArmed >= 0) {
                confirmArmed(); // Enter confirms
            } else if (fHover >= 0) {
                if (kMenuItems[fHover].type == MIT_NUMERIC) {
                    // Enter on unarmed numeric  -  arm it
                    fArmed = fHover;
                    // uiScale is the only numeric value that we have in the
                    // menu, otherwise we'd need to select which one here.
                    fArmedOrigVal = fSettings.uiScale;
                    fNeedsFullRedraw = true;
                } else {
                    activate(fHover);
                }
            }
            return true;
        }
        return false; // key not handled, no redraw needed
    }

private:

    int hitTest(float mx, float my) const
    {
        float lx = mx - fMenuX, ly = my - fMenuY;
        if (lx < 0 || lx > fGeom.w || ly < 0 || ly > fGeom.h) return -1;
        for (int i = 0; i < kMenuItemCount; i++) {
            if (ly >= fGeom.itemY[i] && ly < fGeom.itemY[i] + fGeom.itemH[i]) {
                MenuItemType t = kMenuItems[i].type;
                if (t == MIT_NUMERIC || t == MIT_BOOL ||
                    t == MIT_TRISTATE || t == MIT_ACTION) return i;
                return -1;
            }
        }
        return -1;
    }

    void activate(int row)
    {
        if (row < 0 || row >= kMenuItemCount) return;
        const MenuItemDef &it = kMenuItems[row];
        switch (it.type) {
            case MIT_BOOL:
                switch (it.id) {
                    case MENU_SHOW_INDEX:
                        fSettings.showIndex = !fSettings.showIndex;
                        break;
                    case MENU_SHOW_ARC:
                        fSettings.showArc = !fSettings.showArc;
                        break;
                    case MENU_SNAP_SCALE:
                        fSettings.snapScale = !fSettings.snapScale;
                        fNeedsFullRedraw = true;
                        break;
                    case MENU_GHOST_MODE:
                        fSettings.ghostMode = !fSettings.ghostMode;
                        break;
                    default:
                        break;
                }
                fOnSettingsChanged(fSettings);
                if (it.id == MENU_SHOW_INDEX || it.id == MENU_SHOW_ARC)
                    fNeedsFullRedraw = true;
                break;
            case MIT_TRISTATE:
                if (it.id == MENU_SHOW_VALUE) {
                    fSettings.showValue =
                        (fSettings.showValue == SHOW_NONE)    ? SHOW_STEPPED :
                        (fSettings.showValue == SHOW_STEPPED) ? SHOW_ALL
                                                              : SHOW_NONE;
                    fOnSettingsChanged(fSettings);
                    fNeedsFullRedraw = true;
                }
                break;
            case MIT_ACTION:
                if (it.id == MENU_ABOUT) {
                    fMenuOpen = false;
                    fSplashOpen = true;
                    fNeedsFullRedraw = true;
                }
                break;
            default: break;
        }
    }

    void adjustNumeric(int row, float delta)
    {
        if (row < 0 || row >= kMenuItemCount) return;
        float oldVal = fSettings.uiScale;
        applyNumeric(kMenuItems[row].id,
                     fSettings.uiScale + delta * MENU_STEP,
                     false); // don't notify until confirmed
        if (fSettings.uiScale != oldVal)
            fNeedsFullRedraw = true;
    }

    // Apply a numeric value to settings. notify=false updates the in-memory
    // value only (for live preview during editing); notify=true also calls
    // fOnSettingsChanged which writes the file.
    void applyNumeric(MenuItemId id, float newVal, bool notify = true)
    {
        newVal = std::round(newVal / MENU_STEP) * MENU_STEP;
        if (id == MENU_UI_SCALE) {
            float clamped = std::max(UI_SCALE_MIN,
                                     std::min(UI_SCALE_MAX, newVal));
            if (clamped != fSettings.uiScale) {
                fSettings.uiScale = clamped;
                fNeedsFullRedraw = true;
            }
            if (notify) fOnSettingsChanged(fSettings);
        }
    }

    // Confirm the armed numeric item  -  disarm and write settings.
    void confirmArmed()
    {
        if (fArmed < 0) return;
        fOnSettingsChanged(fSettings);
        fArmed = -1;
        fNeedsFullRedraw = true;
    }

    void indicatorText(const NanoWidgets &wc, float menuW,
                       float midY, const char *txt) const
    {
        const float tx = menuW - MENU_PAD_X - wc.textMeasure(txt);
        wc.setFillColor(COL_MENU_ITEM);
        wc.textLeft(tx, midY, txt);
    }

    // Armed (reverse-video) value field for numeric items being edited
    void indicatorArmed(const NanoWidgets &wc, float menuW,
                        float iy, float ih, float midY, const char *txt) const
    {
        NanoVG &vg = wc.getVG();
        const float tw   = wc.textMeasure(txt);
        const float th   = MENU_FONT_SIZE; // approximate height for pill sizing
        const float pad  = MENU_ARMED_PAD;
        const float rw   = tw + pad * 2.0f;
        const float rx   = menuW - MENU_PAD_X - rw;
        const float ry   = iy + (ih - th) / 2.0f;
        // Light background pill
        wc.setFillColor(COL_MENU_ARMED_BG);
        vg.beginPath();
        vg.rect(rx, ry, rw, th);
        vg.fill();
        // Dark text
        wc.setFillColor(COL_MENU_ARMED_TEXT);
        wc.textLeft(rx + pad, midY, txt);
    }

    // Single tick  -  used for MIT_BOOL on and MIT_TRISTATE SHOW_STEPPED
    void indicatorTick(const NanoWidgets &wc, float menuW,
                       float iy, float ih) const
    {
        NanoVG &vg = wc.getVG();
        const float ts  = MENU_TICK_SIZE;
        const float cx2 = menuW - MENU_PAD_X - MENU_TICK_OFFSET;
        const float cy2 = iy + ih / 2.0f;
        // Tick shape ratios defined in UIConstants as
        // MENU_TICK_ARM / _BASE / _TOP
        vg.beginPath();
        vg.moveTo(cx2 - ts,                    cy2);
        vg.lineTo(cx2 - ts * MENU_TICK_ARM,  cy2 + ts * MENU_TICK_BASE);
        vg.lineTo(cx2 + ts * MENU_TICK_BASE, cy2 - ts * MENU_TICK_TOP);
        wc.setStrokeColor(COL_MENU_ITEM);
        vg.strokeWidth(MENU_TICK_W);
        vg.lineCap(NanoVG::ROUND);
        vg.lineJoin(NanoVG::ROUND);
        vg.stroke();
    }

    // Double tick  -  used for MIT_TRISTATE SHOW_ALL.
    void indicatorDoubleTick(const NanoWidgets &wc, float menuW,
                             float iy, float ih) const
    {
        NanoVG &vg = wc.getVG();
        const float ts      = MENU_TICK_SIZE;
        const float spacing = ts * MENU_TICK_SPACING;
        const float cx2     = menuW - MENU_PAD_X - MENU_TICK_OFFSET;
        const float cy2     = iy + ih / 2.0f;
        wc.setStrokeColor(COL_MENU_ITEM);
        vg.strokeWidth(MENU_TICK_W);
        vg.lineCap(NanoVG::ROUND);
        vg.lineJoin(NanoVG::ROUND);
        // Tick shape ratios defined in UIConstants as
        // MENU_TICK_ARM / _BASE / _TOP
        for (int i = 0; i < 2; i++) {
            const float ox = (i - 1) * spacing;
            vg.beginPath();
            vg.moveTo(cx2 + ox - ts,                    cy2);
            vg.lineTo(cx2 + ox - ts * MENU_TICK_ARM,
                      cy2 + ts * MENU_TICK_BASE);
            vg.lineTo(cx2 + ox + ts * MENU_TICK_BASE,
                      cy2 - ts * MENU_TICK_TOP);
            vg.stroke();
        }
    }

    void drawMenu(const NanoWidgets &wc, float scale) const
    {
        NanoVG &vg = wc.getVG();
        vg.save();
        // Snap origin to integer physical pixels to avoid sub-pixel
        // blurring of borders and text when the window is resized.
        const float sx = std::round(fMenuX * scale) / scale;
        const float sy = std::round(fMenuY * scale) / scale;
        vg.translate(sx, sy);

        // Background
        vg.beginPath();
        vg.roundedRect(0, 0, fGeom.w, fGeom.h, MENU_CORNER_R);
        wc.setFillColor(COL_MENU_BG);
        vg.fill();

        // Border
        vg.beginPath();
        vg.roundedRect(MENU_BORDER_W * 0.5f, MENU_BORDER_W * 0.5f,
                       fGeom.w - MENU_BORDER_W, fGeom.h - MENU_BORDER_W,
                       MENU_CORNER_R - MENU_BORDER_W * 0.5f);
        wc.setStrokeColor(COL_MENU_BORDER);
        vg.strokeWidth(MENU_BORDER_W);
        vg.stroke();

        for (int i = 0; i < kMenuItemCount; i++) {
            const MenuItemDef &it = kMenuItems[i];
            const float iy = fGeom.itemY[i], ih = fGeom.itemH[i];
            const float midY = iy + ih / 2.0f; // vertical midpoint of item row

            if (it.type == MIT_SEP) {
                vg.beginPath();
                vg.moveTo(MENU_PAD_X,          iy + ih / 2.0f);
                vg.lineTo(fGeom.w - MENU_PAD_X, iy + ih / 2.0f);
                wc.setStrokeColor(COL_MENU_SEP);
                vg.strokeWidth(MENU_BORDER_W);
                vg.stroke();
                continue;
            }

            // Hover highlight
            const bool interactive = (it.type == MIT_NUMERIC ||
                                      it.type == MIT_BOOL    ||
                                      it.type == MIT_TRISTATE ||
                                      it.type == MIT_ACTION);
            if (interactive && i == fHover) {
                vg.beginPath();
                vg.rect(MENU_HOVER_INSET, iy,
                        fGeom.w - MENU_HOVER_INSET * 2.0f, ih);
                wc.setFillColor(COL_MENU_BG_HOVER);
                vg.fill();
            }

            vg.fontSize(MENU_FONT_SIZE);
            vg.fontFace(it.type == MIT_HEADER
                        ? FONT_NAME_BOLD : FONT_NAME_REGULAR);

            if      (it.type == MIT_HEADER) wc.setFillColor(COL_MENU_HEADER);
            else if (it.type == MIT_INFO)   wc.setFillColor(COL_MENU_INFO);
            else                            wc.setFillColor(COL_MENU_ITEM);

            if (!it.label) continue;

            wc.textLeft(MENU_PAD_X, midY, it.label);

            if (it.type == MIT_NUMERIC) {
                char tmp[32];
                snprintf(tmp, sizeof(tmp), fSettings.snapScale ? "%.2f" :
                                                                 "%.4f",
                                           fSettings.uiScale);
                const std::string vstr = tmp;
                if (i == fArmed)
                    indicatorArmed(wc, fGeom.w, iy, ih, midY, vstr.c_str());
                else
                    indicatorText(wc, fGeom.w, midY, vstr.c_str());
            } else if (it.type == MIT_BOOL) {
                bool val = false;
                switch (it.id) {
                    case MENU_SHOW_INDEX: val = fSettings.showIndex; break;
                    case MENU_SHOW_ARC:   val = fSettings.showArc;   break;
                    case MENU_SNAP_SCALE: val = fSettings.snapScale; break;
                    case MENU_GHOST_MODE: val = fSettings.ghostMode; break;
                    default: break;
                }
                if (val) indicatorTick(wc, fGeom.w, iy, ih);
                else     indicatorText(wc, fGeom.w, midY, "-");
            } else if (it.type == MIT_TRISTATE) {
                switch (fSettings.showValue) {
                    case SHOW_NONE:
                        indicatorText(wc, fGeom.w, midY, "-");
                        break;
                    case SHOW_STEPPED:
                        indicatorTick(wc, fGeom.w, iy, ih);
                        break;
                    default:
                        indicatorDoubleTick(wc, fGeom.w, iy, ih);
                        break;
                }
            }
        }
        vg.restore();
    }

    void drawSplash(const NanoWidgets &wc, float canvasW, float canvasH,
                    float titleFontSize) const
    {
        NanoVG &vg = wc.getVG();
        const float sw = SPLASH_W, sh = SPLASH_H;
        const float sx = (canvasW - sw) / 2.0f, sy = (canvasH - sh) / 2.0f;

        // Dim overlay
        vg.beginPath();
        vg.rect(0, 0, canvasW, canvasH);
        vg.fillColor(0.0f, 0.0f, 0.0f, SPLASH_DIM_ALPHA);
        vg.fill();

        // Panel background
        vg.beginPath();
        vg.roundedRect(sx, sy, sw, sh, MENU_CORNER_R);
        wc.setFillColor(COL_MENU_BG);
        vg.fill();

        // Panel border
        vg.beginPath();
        vg.roundedRect(sx + MENU_BORDER_W * 0.5f, sy + MENU_BORDER_W * 0.5f,
                       sw - MENU_BORDER_W, sh - MENU_BORDER_W,
                       MENU_CORNER_R - MENU_BORDER_W * 0.5f);
        wc.setStrokeColor(COL_MENU_BORDER);
        vg.strokeWidth(MENU_BORDER_W);
        vg.stroke();

        // Plugin name (title font)
        vg.fontSize(titleFontSize);
        vg.fontFace(FONT_NAME_BOLD);
        wc.setFillColor(COL_MENU_ITEM);
        wc.textCentred(sx + sw / 2.0f, sy + SPLASH_TITLE_Y, PLUGIN_NAME);

        // Description
        vg.fontSize(MENU_FONT_SIZE);
        vg.fontFace(FONT_NAME_REGULAR);
        wc.setFillColor(COL_MENU_HEADER);
        wc.textCentred(sx + sw / 2.0f, sy + SPLASH_DESC_Y, PLUGIN_DESCRIPTION);
        wc.textCentred(sx + sw / 2.0f, sy + SPLASH_DESC_Y + SPLASH_TEXT_LINE_H,
                       fVersion.c_str());

        // Separator line
        vg.beginPath();
        vg.moveTo(sx + MENU_PAD_X,      sy + SPLASH_SEP_Y);
        vg.lineTo(sx + sw - MENU_PAD_X, sy + SPLASH_SEP_Y);
        wc.setStrokeColor(COL_MENU_SEP);
        vg.strokeWidth(MENU_BORDER_W);
        vg.stroke();

        // Splash text lines
        vg.fontSize(MENU_HINT_SIZE);
        vg.fontFace(FONT_NAME_REGULAR);
        wc.setFillColor(COL_MENU_HEADER);
        float textY = sy + SPLASH_TEXT_Y;
        for (const char * const *line = SPLASH_TEXT; *line; ++line) {
            wc.textCentred(sx + sw / 2.0f, textY, *line);
            textY += SPLASH_TEXT_LINE_H;
        }

        // Second separator, after splash text
        const float sep2Y = textY + SPLASH_SEP2_PAD;
        vg.beginPath();
        vg.moveTo(sx + MENU_PAD_X,      sep2Y);
        vg.lineTo(sx + sw - MENU_PAD_X, sep2Y);
        wc.setStrokeColor(COL_MENU_SEP);
        vg.strokeWidth(MENU_BORDER_W);
        vg.stroke();

        // Close hint
        vg.fontSize(MENU_HINT_SIZE);
        vg.fontFace(FONT_NAME_REGULAR);
        wc.setFillColor(COL_MENU_INFO);
        wc.textCentred(sx + sw / 2.0f, sep2Y + SPLASH_CLOSE_PAD,
                       SPLASH_CLOSE_HINT_TEXT);
    }
};
