/*
 * UIPage.h  -  Page rendering and selection engine for MiMi-d UI
 *
 * The Page class owns all page-related state:
 *   - The PageLayout vector (all pages including the common page 0)
 *   - UIGeometry (which also provides the knob grid origin)
 *   - Current page index
 *   - Per-page parameter selection and navigation
 *   - Per-parameter dirty flags for partial redraws
 *
 * A reference to fParamValues (owned by MiMiUI, updated by the host) is
 * stored at construction and used for all drawing and display operations.
 * fParamMeta and fScaleLabels are references into MiMiUI::fParamTables,
 * bound at construction and used for metadata and value formatting.
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
#include <utility>
#include <cmath>
#include <cstring>
#include <algorithm>

#include "UILayout.h"
#include "UIConstants.h"
#include "CairoWidgets.h"
#include "UIParam.h"

// -----------------------------------------------------------------------------
// Page
// -----------------------------------------------------------------------------

class Page {
    // -- Nested types ------------------------------------------------------

    struct KnobLayout {
        float              cx = 0.0f, cy = 0.0f;
        const ParamWidget *pw        = nullptr;
        // fPages index (0=common, 1+=switchable); -1=not on any page
        int                pageIndex = -1;
    };

    struct HitKnob {
        int paramNo;
        float cx, cy;
    };

    struct NavNeighbours {
        int left, right, up, down; // paramNo or -1
    };

    // -- Members -----------------------------------------------------------

    std::vector<PageLayout>          fPages;
    UIGeometry                       fGeometry    = {};
    GridOrigin                       fOrigin      = {};
    const ParamMeta                 (&fParamMeta)[PARAM_COUNT];
    const std::vector<std::string>  (&fScaleLabels)[SP_COUNT];
    int                              fCurrentPage = 1;
    const float                     *fParamValues = nullptr;
    int                     fSelectedParam[3]     = {-1, -1, -1};
    int                     fLastSelectedParam[3] = {-1, -1, -1};
    bool                    fDirtyParam[PARAM_COUNT] = {};

    // Knob canvas position, widget pointer, and page membership for each param.
    // Populated by buildKnobInfo() at construction. Kept separate from
    // fParamMeta so ParamMeta remains pure read-only engine metadata.
    KnobLayout fKnobLayout[PARAM_COUNT];

    // Flat list of knob centres per page combination, built once at init.
    // hitTest() iterates this directly  -  no module tree traversal at runtime.
    std::vector<HitKnob> fHitKnobs[2]; // indexed by fCurrentPage - 1

    // Navigation neighbour tables  -  built once at init.
    // For each switchable page (index 0=page1, 1=page2), every visible param
    // has its best neighbour in each direction pre-computed.
    // Page 0 (common strip) is always included in both tables.
    NavNeighbours fNavNeighbours[2][PARAM_COUNT];
    int           fFirstParam[2] = {-1, -1}; // first visible param per table

public:
    Page(std::vector<PageLayout> pages,
         const UIGeometry &geometry, const float *paramValues,
         const ParamTables &pt)
        : fPages(std::move(pages)),
          fGeometry(geometry),
          fOrigin(geometry.gridOrigin()),
          fParamMeta(pt.paramMeta),
          fScaleLabels(pt.scaleLabels),
          fParamValues(paramValues)
    {
        fSelectedParam[0] = fSelectedParam[1] = fSelectedParam[2] = -1;
        fLastSelectedParam[0] = fLastSelectedParam[1] = fLastSelectedParam[2] = -1;
        std::memset(fDirtyParam, 0, sizeof(fDirtyParam));
        buildKnobInfo();
        buildHitKnobs();
        buildNavNeighbours();
    }

    // -- Page switching ----------------------------------------------------

    int  currentPage()     const { return fCurrentPage; }
    void switchPage(int n)       { fCurrentPage = n; }

    // -- Selection ---------------------------------------------------------

    // Selected parameter on current page (-1 = none)
    int  selectedParam()           const { return fSelectedParam[fCurrentPage]; }
    // Selected parameter on a specific page index (0, 1, or 2)
    int  selectedParam(int idx)    const { return fSelectedParam[idx]; }
    int  lastSelectedParam()       const { return fLastSelectedParam[fCurrentPage]; }

    void select(int paramNo)
    {
        fLastSelectedParam[fCurrentPage] = fSelectedParam[fCurrentPage];
        fSelectedParam[fCurrentPage] = paramNo;
    }

    void clearSelection()
    {
        fLastSelectedParam[fCurrentPage] = fSelectedParam[fCurrentPage];
        fSelectedParam[fCurrentPage] = -1;
    }

    void restoreSelection()
    {
        fSelectedParam[fCurrentPage] = fLastSelectedParam[fCurrentPage];
    }

    // -- Glow colour -------------------------------------------------------

    RGBA glowColour() const
    {
        return (fCurrentPage == 1) ? COL_KNOB_GLOW : COL_KNOB_GLOW_P2;
    }

    // -- Dirty flags -------------------------------------------------------

    bool isDirty(int paramNo)       const { return fDirtyParam[paramNo]; }
    void setDirty(int paramNo)            { fDirtyParam[paramNo] = true; }
    void clearDirty(int paramNo)          { fDirtyParam[paramNo] = false; }
    void clearAllDirty()
    {
        std::memset(fDirtyParam, 0, sizeof(fDirtyParam));
    }

    // -- Hit testing -------------------------------------------------------

    int hitTest(float mx, float my) const
    {
        const float r = fGeometry.trackR + KNOB_HIT_MARGIN;
        const float tRSq = r * r;
        for (const auto &hk : fHitKnobs[fCurrentPage - 1]) {
            float dx = mx - hk.cx, dy = my - hk.cy;
            if (dx*dx + dy*dy <= tRSq) return hk.paramNo;
        }
        return -1;
    }

    // -- Navigation --------------------------------------------------------

    // Spatial arrow-key navigation  -  O(1) direct lookup into the neighbour
    // table built at init. No iteration or arithmetic at nav time.
    void navigate(uint32_t key)
    {
        int &sel = fSelectedParam[fCurrentPage];
        // neighbour table index (0=page1, 1=page2)
        const int pi = fCurrentPage - 1;

        if (sel < 0) {
            sel = fFirstParam[pi];
            return;
        }

        const NavNeighbours &n = fNavNeighbours[pi][sel];
        int next = -1;
        switch (key) {
            case kKeyLeft:  next = n.left;  break;
            case kKeyRight: next = n.right; break;
            case kKeyUp:    next = n.up;    break;
            case kKeyDown:  next = n.down;  break;
        }
        if (next >= 0) sel = next;
    }

    // -- Drawing -----------------------------------------------------------

    // Full redraw of page 0 + current page.
    void draw(const CairoWidgets &wc) const
    {
        const int sel = fSelectedParam[fCurrentPage];
        drawPage(wc, fPages[0], sel, glowColour());
        if (fCurrentPage < (int)fPages.size())
            drawPage(wc, fPages[fCurrentPage], sel, glowColour());
    }

    // Partial redraw  -  redraws knobs with dirty flags set, clears flags.
    // Only redraws params that are on a currently visible page (page 0 or
    // fCurrentPage)  -  params on the hidden page are skipped so their knob
    // position on the surface is not overwritten.
    void drawDirtyKnobs(const CairoWidgets &wc)
    {
        for (int i = 0; i < PARAM_COUNT; i++) {
            if (!fDirtyParam[i]) continue;
            fDirtyParam[i] = false;
            const int pi = fKnobLayout[i].pageIndex;
            if (pi != 0 && pi != fCurrentPage) continue;
            drawKnobForParam(wc, i);
        }
    }

    // -- Display string building -------------------------------------------

    // Build name/value strings for the parameter display panel.

    std::string getSelectedParamName() const
    {
        const int sel = fSelectedParam[fCurrentPage];
        if (sel < 0) return {};

        const ParamMeta &m = fParamMeta[sel];
        const char *grp = m.groupName;
        std::string name = (grp && grp[0] != '\0')
            ? std::string(grp) + " " + m.uiName
            : m.uiName;

        return name;
    }

    std::string getSelectedParamValue() const
    {
        const int sel = fSelectedParam[fCurrentPage];
        if (sel < 0) return {};

        const ParamMeta &m = fParamMeta[sel];
        float val = fParamValues[sel];
        std::string value;
        if (m.spId == SP_NONE) { // Continuous parameters (floats)
            char buf[32];
            snprintf(buf, sizeof(buf), "%.2f", val);
            value = buf;
        } else if (!fScaleLabels[m.spId].empty()) { // enums
            int idx = (int)std::round(
                (val - m.min) / (m.max - m.min) * (float)m.steps);
            idx = std::max(0, std::min(m.steps, idx));
            const char *lbl = fScaleLabels[m.spId][idx].c_str();
            while (*lbl == ' ') lbl++;
            value = lbl;
        } else { // Integer parameters
            value = std::to_string((int)std::round(val));
        }
        return value;
    }

    // -- Page layout access ------------------------------------------------

    const std::vector<PageLayout> &pages() const { return fPages; }

private:
    // -- Private methods ---------------------------------------------------

    // Mirrors drawModule's grid traversal. Populates fKnobLayout[i]
    // for every param.
    void buildKnobInfo()
    {
        for (int i = 0; i < PARAM_COUNT; i++)
            fKnobLayout[i] = {};

        for (int pi = 0; pi < (int)fPages.size(); pi++) {
            for (auto &mod : fPages[pi].modules) {
                int cols = mod.width;

                int col = mod.gridCol, row = mod.gridRow, slot = 0;
                for (const auto &pw : mod.params) {
                    float cx, cy;
                    gridToCX(col, row, fOrigin, fGeometry, cx, cy);
                    if (pw.paramNo >= 0) {
                        fKnobLayout[pw.paramNo].cx        = cx;
                        fKnobLayout[pw.paramNo].cy        = cy;
                        fKnobLayout[pw.paramNo].pw        = &pw;
                        fKnobLayout[pw.paramNo].pageIndex = pi;
                    }
                    advanceGrid(cols, mod.gridCol, col, row, slot);
                }
            }
        }
    }

    // Collect visible param numbers for a page combination.
    // Page 0 (common strip) is always included; switchPage is fPages
    // index 1 or 2.
    std::vector<int> collectVisible(int switchPage) const
    {
        std::vector<int> out;
        for (int pi : {0, switchPage}) {
            if (pi >= (int)fPages.size()) continue;
            for (const auto &mod : fPages[pi].modules)
                for (const auto &pw : mod.params)
                    if (pw.paramNo >= 0) out.push_back(pw.paramNo);
        }
        return out;
    }

    void buildHitKnobs()
    {
        for (int ti = 0; ti < 2; ti++) {
            fHitKnobs[ti].clear();
            for (int paramNo : collectVisible(ti + 1))
                fHitKnobs[ti].push_back({paramNo,
                                         fKnobLayout[paramNo].cx,
                                         fKnobLayout[paramNo].cy});
        }
    }

    void buildNavNeighbours()
    {
        const float rowTol = fGeometry.rowPitch * 0.5f;
        const float colTol = fGeometry.kpitch   * 0.5f;

        for (int ti = 0; ti < 2; ti++) {
            const std::vector<int> visible = collectVisible(ti + 1);

            fFirstParam[ti] = visible.empty() ? -1 : visible[0];

            for (int i = 0; i < PARAM_COUNT; i++)
                fNavNeighbours[ti][i] = {-1, -1, -1, -1};

            for (int src : visible) {
                const float srcCX = fKnobLayout[src].cx;
                const float srcCY = fKnobLayout[src].cy;

                int   bestL = -1, bestR = -1, bestU = -1, bestD = -1;
                // 1e18 = "no candidate yet"
                float distL = 1e18f, distR = 1e18f, distU = 1e18f, distD = 1e18f;

                for (int dst : visible) {
                    if (dst == src) continue;
                    float dx = fKnobLayout[dst].cx - srcCX;
                    float dy = fKnobLayout[dst].cy - srcCY;

                    // 2.0 px minimum displacement threshold  -  well below the
                    // minimum inter-knob grid distance, ensuring only genuinely
                    // directionally-offset knobs qualify as neighbours.
                    if (dx < -2.0f && std::abs(dy) <= rowTol && -dx < distL) {
                        distL = -dx;
                        bestL = dst;
                    }
                    if (dx >  2.0f && std::abs(dy) <= rowTol &&  dx < distR) {
                        distR =  dx;
                        bestR = dst;
                    }
                    if (dy < -2.0f && std::abs(dx) <= colTol && -dy < distU) {
                        distU = -dy;
                        bestU = dst;
                    }
                    if (dy >  2.0f && std::abs(dx) <= colTol &&  dy < distD) {
                        distD =  dy;
                        bestD = dst;
                    }
                }

                fNavNeighbours[ti][src] = {bestL, bestR, bestU, bestD};
            }
        }
    }

    // -- Module and page drawing -------------------------------------------

    void drawModule(const CairoWidgets &wc,
                    const ModuleLayout &mod,
                    int selectedParam, RGBA glowColour) const
    {
        const UIGeometry &g = wc.geometry();

        int cols = mod.width;

        // Collect actual (cx,cy) for each real param and record min/max col/row
        // for frame computation
        struct KnobPos { float cx, cy; int col, row; };
        std::vector<KnobPos> positions;

        int col = mod.gridCol, row = mod.gridRow, slotInRow = 0;
        int minCol = col, maxCol = col, minRow = row, maxRow = row;

        for (const auto &pw : mod.params) {
            float cx, cy;
            gridToCX(col, row, fOrigin, g, cx, cy);
            if (pw.paramNo >= 0) {
                positions.push_back({cx, cy, col, row});
                minCol = std::min(minCol, col);
                maxCol = std::max(maxCol, col);
                minRow = std::min(minRow, row);
                maxRow = std::max(maxRow, row);
            }
            advanceGrid(cols, mod.gridCol, col, row, slotInRow);
        }

        if (positions.empty()) return;

        // -- Draw frame -----------------------------------------------------
        // Frame left/right based on outermost knob columns
        // Frame top/bottom based on outermost knob rows
        float firstCX, firstCY, lastCX, lastCY;
        gridToCX(minCol, minRow, fOrigin, g, firstCX, firstCY);
        gridToCX(maxCol, maxRow, fOrigin, g, lastCX,  lastCY);
        float fx = firstCX - g.kpitch/2.0f + FRAME_GAP_H/2.0f;
        float fw = (lastCX + g.kpitch/2.0f) - fx - FRAME_GAP_H/2.0f;
        float fy = g.frameTop(firstCY);
        // Frame extends down to bottom of last row
        float fh = g.frameBot(lastCY) - fy;
        wc.drawGroupFrame(fx, fy, fw, fh, mod.title.c_str());

        // -- Draw knobs -----------------------------------------------------
        size_t posIdx = 0;
        for (const auto &pw : mod.params) {
            if (posIdx >= positions.size()) break;
            const KnobPos &kp = positions[posIdx++];
            float val = fParamValues[pw.paramNo];

            // Normalise value
            float norm = (pw.maxVal > pw.minVal)
                ? std::max(0.0f, std::min(1.0f,
                      (val - pw.minVal) / (pw.maxVal - pw.minVal)))
                : 0.0f;
            const std::string valueTxt = pw.formatValue(val);

            // Bipolar if range straddles zero
            float zeroNorm = (pw.minVal < 0.0f && pw.maxVal > 0.0f)
                ? (0.0f-pw.minVal)/(pw.maxVal-pw.minVal) : -1.0f;
            wc.drawKnob(kp.cx, kp.cy, norm, pw.steps, pw.isInteger,
                        pw.name.c_str(), valueTxt.c_str(), zeroNorm,
                        pw.paramNo == selectedParam, true,
                        glowColour);
            if (!pw.symPoints.empty())
                wc.drawSymbols(kp.cx, kp.cy, pw.symPoints);
        }
    }

    void drawPage(const CairoWidgets &wc,
                  const PageLayout &page,
                  int selectedParam, RGBA glowColour) const
    {
        for (const auto &mod : page.modules)
            drawModule(wc, mod, selectedParam, glowColour);
    }

    // -- Single knob partial redraw ----------------------------------------
    void drawKnobForParam(const CairoWidgets &wc, int paramNo) const
    {
        const KnobLayout  &kl = fKnobLayout[paramNo];
        if (!kl.pw) return;
        const ParamWidget &pw = *kl.pw;

        // Clear the knob bounding box to the background colour before
        // redrawing, so glow rings and shadows from the previous state
        // are fully erased regardless of what changed.
        wc.clearKnobArea(kl.cx, kl.cy);

        float val = fParamValues[paramNo];
        float norm = (pw.maxVal > pw.minVal)
            ? std::max(0.0f, std::min(1.0f,
                  (val - pw.minVal) / (pw.maxVal - pw.minVal)))
            : 0.0f;

        const std::string valueTxt = pw.formatValue(val);

        float zeroNorm = (pw.minVal < 0.0f && pw.maxVal > 0.0f)
            ? (0.0f-pw.minVal)/(pw.maxVal-pw.minVal) : -1.0f;

        // The selection is always owned by the current page's slot, regardless
        // of which visual strip the knob belongs to  -  there is only ever one
        // selected param at a time across both visible strips.
        bool selected = (paramNo == fSelectedParam[fCurrentPage]);

        wc.drawKnob(kl.cx, kl.cy, norm, pw.steps, pw.isInteger,
                    pw.name.c_str(), valueTxt.c_str(), zeroNorm, selected,
                    false, // drawName  -  static, skip on partial redraw
                    glowColour());
    }
};
