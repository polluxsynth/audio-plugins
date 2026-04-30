/*
 * MiMiUI.cpp   -   DISTRHO UI for MiMi-d synthesizer
 *
 * Build requirements:
 *   - DISTRHO_UI_USE_NANOVG=1
 *   - Link against OpenGL
 *
 * The UI draws in logical units (UI_W x UI_H, derived from UI_SIZE in
 * UILayoutDefs.h). NanoVG renders into an off-screen FBO which is blitted
 * to the screen on every onNanoDisplay() call.  The FBO is only re-rendered
 * when fRedrawLevel != None (i.e. something has changed).
 *
 * UI settings are read from ~/.config/pollux/mimid/ui-settings at
 * startup (scale, knob display flags, ghost mode).
 *
 * Fonts "regular" and "bold" are embedded as static arrays
*  (NotoSans-Regular.h, NotoSans-Bold.h) and loaded via createFontFromMemory().
 *
 * Interaction:
 *   - Left-click + drag up/down on any knob changes its value.
 *   - Scroll wheel on a knob nudges the value.
 *   - Clicking a page button switches pages.
 *   - Double-click on a knob resets to default (TODO).
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

#include "DistrhoUI.hpp"
#include "Engine/ParamsEnum.h"   // Parameters enum (VOLUME, CUTOFF, ..., PARAM_COUNT)

#include "UI/UIConstants.h"
#include "UI/UIGeometry.h"
#include "UI/UISettings.h"
#include "UI/NanoWidgets.h"
#include "UI/UIMenu.h"
#include "UI/UILayout.h"
#include "UI/UIParam.h"
#include "UI/UIPage.h"
#include "UI/UILayoutDefs.h"     // for UI_W / UI_H via UI_SIZE macro

#include "UI/NotoSans-Regular.h"
#include "UI/NotoSans-Bold.h"

#include <vector>
#include <string>
#include <chrono>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <sys/stat.h>

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------------
// Parameter metadata tables  -  built once from ParamDefs.h
//
// We include ParamDefs.h a second time here (after ParamsEnum.h has already
// included it) with fresh macro definitions to populate our own tables.
// This is exactly how MiMi-d.cpp builds its parameter registration tables.
// -----------------------------------------------------------------------------

static ParamTables buildMetaTables()
{
    ParamTables t;
    std::fill(std::begin(t.groupName), std::end(t.groupName), nullptr);
    float gSpMin[SP_COUNT] = {};
    float gSpMax[SP_COUNT] = {};

#define PARAMPOINTS(SPID, FIRST, ...) \
    { std::vector<std::string> tmp = { __VA_ARGS__ }; \
      t.scaleLabels[SPID] = tmp; \
      gSpMin[SPID] = (float)(FIRST); \
      gSpMax[SPID] = (float)(FIRST) + (float)tmp.size() - 1.0f; }
#define PARAMHINTS(SPID, HINTS)   /* not needed here */
#define PARAMGROUP(PGID, NAME, SYMBOL) \
    t.groupName[PGID] = NAME;
#define SP_MIN 0
#define SP_MAX 0
#define PARAM(PARAMNO, PG, SP, NAME, SYMBOL, MIN, MAX, DEFAULT, SETFUNC) \
    t.paramMeta[PARAMNO].name      = NAME; \
    t.paramMeta[PARAMNO].uiName    = NAME; \
    t.paramMeta[PARAMNO].groupName = t.groupName[PG]; \
    t.paramMeta[PARAMNO].spId  = SP; \
    t.paramMeta[PARAMNO].min   = (SP == SP_NONE || t.scaleLabels[SP].empty()) \
                                 ? (float)(MIN) : gSpMin[SP]; \
    t.paramMeta[PARAMNO].max   = (SP == SP_NONE || t.scaleLabels[SP].empty()) \
                                 ? (float)(MAX) : gSpMax[SP]; \
    t.paramMeta[PARAMNO].def   = (float)(DEFAULT); \
    t.paramMeta[PARAMNO].steps = (!t.scaleLabels[SP].empty()) \
                                 ? (int)t.scaleLabels[SP].size() - 1 \
                                 : (SP != SP_NONE) \
                                 ? (int)((float)(MAX) - (float)(MIN)) \
                                 : 0;
#include "Engine/ParamDefs.h"
#undef SP_MIN
#undef SP_MAX
    return t;
}

// -----------------------------------------------------------------------------
// Build ParamWidget from metadata table
// -----------------------------------------------------------------------------

static ParamWidget makeParam(int paramNo,
                             const ParamMeta (&paramMeta)[PARAM_COUNT],
                             const std::vector<std::string> (&scaleLabels)[SP_COUNT])
{
    const ParamMeta &m = paramMeta[paramNo];
    ParamWidget pw;
    pw.paramNo     = paramNo;
    pw.minVal      = m.min;
    pw.maxVal      = m.max;
    pw.defVal      = m.def;
    pw.scaleLabels = (m.spId == SP_NONE) ? std::vector<std::string>()
                                         : scaleLabels[m.spId];

    // steps: number of intervals = number of labels - 1.
    // For SP_NONE (continuous) scaleLabels is empty -> steps = 0.
    // For SP_INTS / SP_HIDDEN (hints, not real scale points) also empty -> 0.
    pw.steps = pw.scaleLabels.empty()
        ? 0
        : (int)pw.scaleLabels.size() - 1;

    // isInteger: SP_INTS/SP_HIDDEN have a non-NONE spId but no scale labels
    pw.isInteger = (m.spId != SP_NONE && pw.scaleLabels.empty());

    // Name: initially from ParamDefs.h; may be overridden per placement.
    pw.name = m.name;

    return pw;
}

// -----------------------------------------------------------------------------
// Build PageLayout vector from UILayoutDefs.h
// Two passes over UILayoutDefs.h: pass 1 declares page stubs via
// UILAYOUT_PAGE, pass 2 pushes modules and params into the pages vector.
// -----------------------------------------------------------------------------

static std::vector<PageLayout> buildPages(ParamTables &pt)
{
    std::vector<PageLayout> pages;

    // Pass 1  -  page declarations
#define UILAYOUT_PAGE(ID, LABEL) \
    { PageLayout p; p.label = LABEL; pages.push_back(p); }

#include "UI/UILayoutDefs.h"

    // Pass 2  -  modules and parameters pushed directly into pages.
    // UILAYOUT_PARAM_POINTS preceding a UILAYOUT_PARAM stashes symbol
    // points in _pendingSymPoints; UILAYOUT_PARAM moves them into the
    // widget and clears the staging vector.
    ModuleLayout *cur = nullptr;
    std::vector<std::pair<float,Symbol>> _pendingSymPoints;

#define UILAYOUT_PAGE(ID, LABEL)   /* already done */
#define UILAYOUT_COMMON_START      /* skip */
#define UILAYOUT_COMMON_END        /* skip */
#define UILAYOUT_MODULE(PAGEID, PGID, DIR, GCOL, GROW, ...) \
    if ((PAGEID) < (int)pages.size()) { \
        const char *_ovr[] = { __VA_ARGS__ }; \
        const char *title = (sizeof(_ovr) / sizeof(_ovr[0]) > 0) \
            ? _ovr[0] : pt.groupName[PGID]; \
        pages[PAGEID].modules.push_back({title ? title : #PGID, \
                                         (DIR), \
                                         GCOL, GROW, {}}); \
        cur = &pages[PAGEID].modules.back(); \
    }
#define UILAYOUT_MODULE_END \
    cur = nullptr;
#define UILAYOUT_PARAM_POINTS(...) \
    { \
        const int _all[] = { __VA_ARGS__ }; \
        const int _n = (int)(sizeof(_all)/sizeof(_all[0])); \
        for (int _i = 0; _i + 1 < _n; _i += 2) \
            _pendingSymPoints.push_back( \
                {(float)_all[_i], (Symbol)_all[_i+1]}); \
    }
#define UILAYOUT_PARAM(PARAMNO, ...) \
    if (cur) { \
        auto _pw = makeParam(PARAMNO, pt.paramMeta, pt.scaleLabels); \
        const char *_ovr[] = { __VA_ARGS__ }; \
        if (sizeof(_ovr) / sizeof(_ovr[0]) > 0) { \
            _pw.name = _ovr[0]; \
            pt.paramMeta[PARAMNO].uiName = _ovr[0]; \
        } \
        _pw.symPoints = std::move(_pendingSymPoints); \
        cur->params.push_back(std::move(_pw)); \
    }

#define WIDTH(n) (n)
#define VERT     WIDTH(1)
#define HORIZ    WIDTH(16)

#include "UI/UILayoutDefs.h"

#undef HORIZ
#undef VERT
#undef WIDTH
#undef UILAYOUT_PARAM_POINTS

    return pages;
}

// -----------------------------------------------------------------------------
// UI settings  -  global instance, file I/O
// -----------------------------------------------------------------------------



// Settings file format: "%.2f %d %d %d %d %d\n"
// 6 fields, worst-case ~8 chars each (e.g. "-1.00  ").
// Used by both readUISettings and writeUISettings.
static constexpr int SETTINGS_FIELD_COUNT = 6;
static constexpr int SETTINGS_LINE_LEN    = SETTINGS_FIELD_COUNT * 8;

static std::string uiSettingsPath()
{
    const char *home = getenv("HOME");
    return std::string(home ? home : "/tmp") + "/.config/pollux/mimid/ui-settings";
}

static UISettings readUISettings()
{
    // Start with defaults  -  sscanf overrides only the fields present
    // in the file, so a missing or truncated file leaves unread fields
    // at their defaults.
    UISettings s;
    float  sc  = s.uiScale;
    int    si  = s.showIndex       ? 1 : 0;
    int    sa  = s.showArc         ? 1 : 0;
    int    sv  = (int)s.showValue;
    int    sr  = s.snapScale       ? 1 : 0;
    int    gm  = s.ghostMode       ? 1 : 0;

    FILE *f = fopen(uiSettingsPath().c_str(), "r");
    if (f) {
        char buf[SETTINGS_LINE_LEN] = {};
        if (fgets(buf, sizeof(buf), f))
            sscanf(buf, "%f %d %d %d %d %d", &sc, &si, &sa, &sv, &sr, &gm);
        fclose(f);
    }

    float uiScale = std::max(UI_SCALE_MIN, std::min(UI_SCALE_MAX, sc));
    s.uiScale   = uiScale;
    s.showIndex = si != 0;
    s.showArc   = sa != 0;
    s.showValue = (sv == 0) ? SHOW_NONE
                : (sv == 1) ? SHOW_STEPPED : SHOW_ALL;
    s.snapScale = sr != 0;
    s.ghostMode = gm != 0;
    return s;
}

static void writeUISettings(const UISettings &s)
{
    // Derive the directory by truncating the path at the last '/'.
    std::string dir = uiSettingsPath();
    const auto lastSlash = dir.rfind('/');
    if (lastSlash != std::string::npos) dir.resize(lastSlash);

    // mkdir -p: walk the path and create each component in turn.
    for (size_t i = 1; i < dir.size(); i++) {
        if (dir[i] == '/') {
            dir[i] = '\0';
            mkdir(dir.c_str(), 0755);
            dir[i] = '/';
        }
    }
    mkdir(dir.c_str(), 0755);

    FILE *f = fopen(uiSettingsPath().c_str(), "w");
    if (!f) return;
    fprintf(f, "%.2f %d %d %d %d %d\n",
            s.uiScale,
            s.showIndex      ? 1 : 0,
            s.showArc        ? 1 : 0,
            (int)s.showValue,
            s.snapScale      ? 1 : 0,
            s.ghostMode      ? 1 : 0);
    fclose(f);
}

// Use a bit of macro magic to determine if we have a PG_REL parameter group.
// When this group is present, the plugin has been compiled in native mode
// with the envelope release parameters in their own group, in order to
// minimize the number of screens in the Zynthian UI. Conversely, when
// there is no PG_REL group, it means that all envelope parameters are
// in their respective groups (PG_FENV or PG_LENV) which facilitates the
// graphic envelope display in Zynthian, but leaves the release parameter
// for each envelope hanging as a lone parameter in the second envelope
// screen. This is indicated by a 'z' appended to the version name.
//
// This distinction could be done at compile time with a simple HAVE_ZYNTHIAN
// macro, but it was felt better to keep ParamDefs.h clean of that type of
// selection macros, and instead detect the presence of PG_REL here.
// Unfortunately due to macro processing limitations, this cannot be done
// at compile time so we resort to this run time function.
bool paramgroups_have_pg_rel()
{
// When we hit a PARAMGROUP(PG_REL, ...) macro call in ParamDefs.h.
// this definition will cause the SECOND macro to return 'true'
#define PG_REL DUMMY, true
// Return second parameter
#define SECOND(A, B, ...) B
// If there is only one parameter to IS_PROBE, it returns 'false'. If there
// are two or more, it returns the second of the two parameters passed in.
// In the case of passing PG_REL above, it will thus return 'true'.
#define IS_PROBE(...) SECOND(__VA_ARGS__, false)
// For every PARAMGROUP, potentially logical or with 'true' if the PGID is
// PG_REL .
#define PARAMGROUP(PGID, NAME, SYMBOL) | IS_PROBE(PGID)

    static constexpr bool has_pg_rel = false
#include "Engine/ParamDefs.h" // Every PARAMGROUP adds '| true' or '| false'
     ; // Terminates the constructed multi-line logical calculation.

#undef PG_REL

    return has_pg_rel;
}

// Create version string, potentially adding 'z' in the case of lacking PG_REL,
// which indicates the plugin is built in 'Zynthian' mode.
std::string makeVersionString()
{
  std::string version = PLUGIN_VERSION;

  if (!paramgroups_have_pg_rel()) version += "z";
  return version;
}

// -----------------------------------------------------------------------------
// MiMiUI
// -----------------------------------------------------------------------------

class MiMiUI : public UI
{
    // -- Members -----------------------------------------------------------
    UISettings   fSettings;        // loaded at construction, updated by menu/resize
    float        fUIScale;         // runtime scale; updated by onResize()
    ParamTables  fParamTables;     // parameter metadata, populated at construction
    const ParamMeta                (&fParamMeta)[PARAM_COUNT];
    const std::vector<std::string> (&fScaleLabels)[SP_COUNT];
    UIGeometry   fGeometry;
    NanoWidgets  fWC;              // widget drawing  -  stores geometry and settings

    float  fParamValues[PARAM_COUNT] = {};

    Page         fPage;            // page rendering, selection, navigation, dirty tracking

    int    fDragParam;
    float  fDragStartY;
    float  fDragStartVal;
    float  fDragLastY;          // last known cursor Y, for modifier re-anchoring
    bool   fPageBtnPressed;

    uint   fCurrentMod;         // live modifier state (Shift/Ctrl), updated by onKeyboard

    ButtonStrip  fStrip;          // cached  -  used for hit testing

    // Pending redraw level  -  always promoted, never demoted.
    // Full    = repaint background, strip, and all knobs
    //           (page switch, init, etc.)
    // Display = redraw full display panel including parameter name
    // Value   = redraw value area of display panel only
    // None    = dirty knobs only (handled separately via fPage dirty flags)
    enum class RedrawLevel { None, Value, Display, Full };
    RedrawLevel fRedrawLevel = RedrawLevel::None;

    UIMenu  fMenu;     // owns all menu and splash state

    // -- Value-update draw rate limiting -----------------------------------
    // Full and Display redraws always execute immediately.
    // Value redraws are throttled to DRAW_FRAME_INTERVAL_MS.
    using Clock = std::chrono::steady_clock;
    Clock::time_point fLastUpdateTime;   // time of last FBO render; default = epoch (far past)
    bool fUpdateDrawPending = false; // true if a parameter update draw was deferred

    // -- Off-screen framebuffer (FBO) ---------------------------------------
    // The UI is rendered into this FBO; onNanoDisplay blits it to the screen.
    // This avoids artefacts from the GL framebuffer being cleared before each
    // onNanoDisplay call (by DPF / the host windowing system).
    //
    // fFBOImage is a nanoVG-managed GL texture (created via createImageFromRGBA
    // with IMAGE_FLIP_Y to compensate for OpenGL Y-inversion).  fGLFBO is a
    // raw GL framebuffer object that renders into that same texture, with
    // fGLStencil providing the stencil attachment nanoVG requires.
    NanoImage fFBOImage;           // nanoVG image handle (owns the GL texture)
    GLuint    fGLFBO     = 0;      // GL framebuffer object
    GLuint    fGLStencil = 0;      // GL stencil renderbuffer
    uint      fFBOWidth  = 0;      // physical pixel dimensions of the FBO
    uint      fFBOHeight = 0;
    bool      fFBOValid  = false;  // false until the first full render pass

public:
    MiMiUI()
        : UI(),
          fSettings(readUISettings()),
          fUIScale(fSettings.uiScale),
          fParamTables(buildMetaTables()),
          fParamMeta(fParamTables.paramMeta),
          fScaleLabels(fParamTables.scaleLabels),
          fGeometry(),
          fWC(fGeometry, fSettings),
          fPage(buildPages(fParamTables),
                fGeometry, fParamValues, fParamTables),
          fDragParam(-1),
          fDragStartY(0.0f),
          fDragStartVal(0.0f),
          fDragLastY(0.0f),
          fPageBtnPressed(false),
          fCurrentMod(0),
          fStrip(),
          fRedrawLevel(RedrawLevel::Full),
          fMenu(fSettings, [this](const UISettings &s) {
              writeUISettings(s);
              fWC.updateSettings(s);
          }, makeVersionString())
    {
        createFontFromMemory(FONT_NAME_REGULAR, NotoSans_Regular_ttf,
                                                sizeof(NotoSans_Regular_ttf),
                                                false);
        createFontFromMemory(FONT_NAME_BOLD,    NotoSans_Bold_ttf,
                                                sizeof(NotoSans_Bold_ttf),
                                                false);

        // Allow the host to resize the window freely; DPF enforces the aspect
        // ratio.  The minimum size is one quarter of the default dimensions.
        setGeometryConstraints(UI_W * UI_SCALE_MIN, UI_H * UI_SCALE_MIN, true);
        setSize(static_cast<uint>(UI_W * fUIScale),
                static_cast<uint>(UI_H * fUIScale));
    }

    ~MiMiUI() override
    {
        // GL context is still current during destruction in DPF/LV2.
        if (fGLFBO)     glDeleteFramebuffers(1,  &fGLFBO);
        if (fGLStencil) glDeleteRenderbuffers(1, &fGLStencil);
        // fFBOImage destructor releases the nanoVG image (and its GL texture).
    }

    // -- Paint -------------------------------------------------------------
    // The UI is always rendered into an off-screen FBO so that DPF/host
    // clearing the GL framebuffer before each call is harmless.  Every
    // onNanoDisplay() blits the FBO to the screen (one textured quad, ~free
    // on the GPU).  The FBO is only re-rendered when fRedrawLevel != None or
    // when the FBO is first created / resized.

private:
    // Allocate (or reallocate) the off-screen FBO at physical pixel size w×h.
    // Must be called while a GL context is current (i.e. from onNanoDisplay).
    void createFBO(uint w, uint h)
    {
        // Release existing GL objects first (order matters: delete the FBO
        // before the texture it references, then let NanoImage release the
        // texture by reassignment below).
        if (fGLFBO) {
            glDeleteFramebuffers(1,  &fGLFBO);
            fGLFBO = 0;
        }
        if (fGLStencil) {
            glDeleteRenderbuffers(1, &fGLStencil);
            fGLStencil = 0;
        }

        // Create (or recreate) the nanoVG-managed texture.
        // IMAGE_FLIP_Y tells nanoVG to flip the image vertically when blitting
        // it back to the screen, compensating for OpenGL's bottom-left origin.
        // DPF asserts data != nullptr, so we pass a zeroed temporary buffer;
        // the contents don't matter since the FBO will overwrite it
        // on first use.
        const size_t bufSize = (size_t)w * h * 4;
        std::vector<uchar> zeroBuf(bufSize, 0);
        fFBOImage = createImageFromRGBA(w, h, zeroBuf.data(),
                                        NanoVG::IMAGE_FLIP_Y);
        const GLuint texId = fFBOImage.getTextureHandle();

        // Build the GL FBO and attach the texture as the colour target.
        glGenFramebuffers(1, &fGLFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, fGLFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, texId, 0);

        // Attach a stencil renderbuffer (nanoVG uses the stencil buffer for
        // its fill rules and scissor clipping).
        glGenRenderbuffers(1, &fGLStencil);
        glBindRenderbuffer(GL_RENDERBUFFER, fGLStencil);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8,
                              (GLsizei)w, (GLsizei)h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, fGLStencil);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        fFBOWidth  = w;
        fFBOHeight = h;
        fFBOValid  = false;
    }

    void requestRedraw(RedrawLevel lvl)
    {
        if (lvl > fRedrawLevel) fRedrawLevel = lvl;
    }

public:
    void onNanoDisplay() override
    {
        const uint physW = getWidth();
        const uint physH = getHeight();
        const float s    = fUIScale;

        // -- Lazy FBO creation / resize ------------------------------------
        if (!fGLFBO || fFBOWidth != physW || fFBOHeight != physH) {
            createFBO(physW, physH);
            fRedrawLevel = RedrawLevel::Full;
        }

        // -- FBO render pass (only when something changed) -----------------
        bool shouldRender = (fRedrawLevel != RedrawLevel::None || !fFBOValid);

        if (shouldRender) {
            // DPF has already called nvgBeginFrame() for us on the default
            // framebuffer. Cancel it via the NanoVG public API, render into
            // our FBO, then open a fresh frame for the blit which DPF's
            // endFrame() will close after we return.
            fWC.setVG(*this);
            NanoVG& vg = fWC.getVG();
            vg.cancelFrame();

            glBindFramebuffer(GL_FRAMEBUFFER, fGLFBO);
            glViewport(0, 0, (GLsizei)physW, (GLsizei)physH);

            vg.beginFrame(physW, physH, 1.0f);
            vg.scale(s, s);

            if (fRedrawLevel == RedrawLevel::Full || !fFBOValid) {
                // Full redraw: clear and repaint background, strip, all knobs.
                // In ghost mode the background and knobs are skipped, but the
                // strip draw must always run to populate the fGeom geometry
                // cache that partial display-panel redraws depend on.
                glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                const std::string &selName = fPage.getSelectedParamName();
                const std::string &selValue = fPage.getSelectedParamValue();

                if (!fSettings.ghostMode) fWC.drawBackground(UI_W, UI_H);
                fStrip.draw(fWC, fPageBtnPressed, fMenu.isOpen(), UI_W,
                            selName.c_str(), selValue.c_str(),
                            fPage.glowColour());
                if (!fSettings.ghostMode) fPage.draw(fWC);
            } else {
                // Partial redraw: repaint dirty knobs over retained
                // FBO contents,
                // and update the display panel if the selected param changed.
                // Knobs on the hidden page are skipped by drawDirtyKnobs; their
                // dirty flags are cleared so they don't accumulate across page
                // toggles (the page toggle triggers a Full redraw anyway).

                fPage.drawDirtyKnobs(fWC);
                if (fRedrawLevel >= RedrawLevel::Display)
                    fStrip.drawDisplay(fWC,
                                       fPage.getSelectedParamName().c_str(),
                                       fPage.getSelectedParamValue().c_str(),
                                       fPage.glowColour());
                else
                    fStrip.drawValue(fWC,
                                     fPage.getSelectedParamValue().c_str(),
                                     fPage.glowColour());
            }

            vg.endFrame();

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, (GLsizei)physW, (GLsizei)physH);

            fFBOValid          = true;
            fRedrawLevel       = RedrawLevel::None;
            fUpdateDrawPending = false;
            fLastUpdateTime    = Clock::now();
            fPage.clearAllDirty();

            // Open the blit frame; DPF calls endFrame() on the NanoBaseWidget
            // after we return, which flushes it to the default framebuffer.
            vg.beginFrame(physW, physH, 1.0f);
        }
        // else: DPF's frame is already open — use it directly for the blit.

        // -- Blit FBO to screen --------------------------------------------
        // fFBOImage was created with IMAGE_FLIP_Y, so imagePattern renders
        // it right-side-up despite OpenGL's bottom-left texture origin.
        const NanoVG::Paint img = imagePattern(
            0.0f, 0.0f, (float)physW, (float)physH, 0.0f, fFBOImage, 1.0f);
        beginPath();
        rect(0.0f, 0.0f, (float)physW, (float)physH);
        fillPaint(img);
        fill();

        // -- Menu / splash overlay (always live, not cached in FBO) --------
        // Drawn in the blit frame so hover state and splash animation update
        // every repaint() without requiring an FBO re-render.
        fWC.setVG(*this);
        fMenu.draw(fWC, UI_W, UI_H, fStrip.titleFontSize(), s);

        if (fMenu.takeNeedsFullRedraw()) {
            requestRedraw(RedrawLevel::Full);
            repaint();
        }
        // DPF calls endFrame() after we return.
    }

    // -- Idle callback / deferred value draw -------------------------------
    void uiIdle() override
    {
        if (!fUpdateDrawPending) return;
        const auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                Clock::now() - fLastUpdateTime).count();
        if (elapsed >= UPDATE_DRAW_INTERVAL_MS)
            repaint();
    }

    // -- Parameter update from host ----------------------------------------
    void parameterChanged(uint32_t index, float value) override
    {
        if (index >= PARAM_COUNT) return;

        // Skip if value hasn't changed  -  avoids unnecessary dirty flags
        // and repaints when hosts re-send the same value, or when stepped
        // parameters are already at a boundary.
        if (value == fParamValues[index]) return;

        fParamValues[index] = value;
        fPage.setDirty(index);
        requestRedraw(RedrawLevel::Value);

        // Suppress display updates while the menu or splash is open.
        // The user cannot see the knobs anyway, and suppressing here
        // avoids the blit cost on every parameter change.  When the
        // menu closes a Full redraw is triggered, so knob state will
        // be correct immediately on dismiss.
        if (fMenu.isOpen() || fMenu.isSplashOpen()) return;

        // Always defer to uiIdle for rate-limiting: never call repaint()
        // directly here.  uiIdle fires frequently enough that the added
        // latency is imperceptible, and it prevents a repaint() storm when
        // the host delivers a burst of automation events back-to-back.
        fUpdateDrawPending = true;
    }

    // -- Resize ------------------------------------------------------------
    void onResize(const ResizeEvent& ev) override
    {
        // Derive the new logical scale from the actual window width.
        // Use the smaller of set width and height so that the UI always
        // fits within the window regardless of which aspect ratio is set.
        // Not throttled to get fluid appearance on resize; uses a bit of
        // CPU due to the full redraw, but we consider resize a fairly
        // unusual operation being used for configuration rather than
        // reuqiring utmost efficiency.
        float wSize = ev.size.getWidth() / (float)UI_W;
        float hSize = ev.size.getHeight() / (float)UI_H;
        float scale = std::min(wSize, hSize);
        if (fSettings.snapScale)
            scale = std::floor(scale / MENU_STEP + .75f) * MENU_STEP;
        fUIScale = fSettings.uiScale = scale;
        fWC.updateSettings(fSettings);
        writeUISettings(fSettings);
        requestRedraw(RedrawLevel::Full);
    }

    // -- Mouse button ------------------------------------------------------
    bool onMouse(const MouseEvent &ev) override
    {
        const float mx = logicalX(ev.pos.getX());
        const float my = logicalY(ev.pos.getY());

        // -- Splash / menu  -  delegate to UIMenu ---------------------------
        if (fMenu.isSplashOpen() || fMenu.isOpen()) {
            bool consumed = fMenu.onMouse(mx, my, ev.press, ev.button);
            if (fMenu.takeNeedsFullRedraw()) requestRedraw(RedrawLevel::Full);
            repaint();
            return consumed;
        }

        // -- Right-click  -  open menu at cursor -----------------------------
        if (ev.button == 2 || ev.button == 3) {
            if (ev.press) {
                fMenu.open(mx, my, UI_W, UI_H);
                requestRedraw(RedrawLevel::Full);
                repaint();
            }
            return true;
        }

        if (ev.button != 1) return false;

        const bool onPageBtn = (
            mx >= fStrip.geometry().pageBtnX &&
            mx <= fStrip.geometry().pageBtnX + fStrip.geometry().pageBtnW &&
            my >= PAGE_BTN_Y && my <= PAGE_BTN_Y + PAGE_BTN_H);
        const bool onUIBtn   = (
            mx >= fStrip.geometry().uiBtnX &&
            mx <= fStrip.geometry().uiBtnX + fStrip.geometry().uiBtnW &&
            my >= PAGE_BTN_Y && my <= PAGE_BTN_Y + PAGE_BTN_H);

        if (!ev.press) {
            if (fDragParam >= 0) {
                editParameter(fDragParam, false);
                fDragParam = -1;
                return true;
            }
            if (fPageBtnPressed) {
                fPageBtnPressed = false;
                if (onPageBtn)
                    fPage.switchPage(fPage.currentPage() == 1 ? 2 : 1);
                requestRedraw(RedrawLevel::Full);
                repaint();
                return true;
            }
            return false;
        }

        // Press: UI button  -  open menu below strip
        if (onUIBtn) {
            float menuX = fStrip.geometry().uiBtnX + fStrip.geometry().uiBtnW;
            float menuY = BTN_STRIP_SEP_Y + MENU_EDGE_MARGIN;
            fMenu.open(menuX, menuY, UI_W, UI_H);
            requestRedraw(RedrawLevel::Full);
            repaint();
            return true;
        }

        // Press: page button
        if (onPageBtn) {
            fPageBtnPressed = true;
            requestRedraw(RedrawLevel::Full);
            repaint();
            return true;
        }

        // Press: knob hit test  -  also selects the parameter
        int param = fPage.hitTest(mx, my);
        if (param >= 0) {
            int prevSel = fPage.selectedParam();
            fPage.select(param);
            fDragParam     = param;
            fDragStartY    = my;
            fDragStartVal  = fParamValues[param];
            editParameter(param, true);
            // Redraw old selection (remove highlight), new selection
            // (add highlight), strip
            if (prevSel >= 0) fPage.setDirty(prevSel);
            fPage.setDirty(param);
            requestRedraw(RedrawLevel::Display);
            repaint();
            return true;
        }

        return false;
    }

    // -- Mouse drag / motion -----------------------------------------------
    bool onMotion(const MotionEvent &ev) override
    {
        const float mx = logicalX(ev.pos.getX());
        const float my = logicalY(ev.pos.getY());

        // Menu / splash
        if (fMenu.isSplashOpen() || fMenu.isOpen()) {
            fMenu.onMotion(mx, my);
            if (fMenu.takeNeedsFullRedraw()) repaint();
            return true;
        }

        if (fDragParam < 0) return false;
        fDragLastY = my;                           // kept current for modifier re-anchor
        const float dy = fDragStartY - my;   // drag up = increase

        float lo = fParamMeta[fDragParam].min;
        float hi = fParamMeta[fDragParam].max;

        // 200 logical units drag = full range (at speed 1.0).
        // fCurrentMod is updated live by onKeyboard so speed changes take
        // effect immediately; re-anchoring happens there on modifier change.
        float speed = speedMult(fDragParam, fCurrentMod);
        float newVal = fDragStartVal + dy / DRAG_FULL_RANGE_PX * speed * (hi - lo);
        newVal = std::max(lo, std::min(hi, newVal));
        newVal = snapToSteps(newVal, lo, hi, fParamMeta[fDragParam].steps);

        // Skip the update if the (possibly snapped) value hasn't moved.
        // This avoids redundant FBO renders on every mouse-move event when
        // dragging a stepped param that's already at a step boundary.
        if (newVal == fParamValues[fDragParam]) return true;

        setParameterValue(fDragParam, newVal);
        parameterChanged(fDragParam, newVal);
        return true;
    }

    // -- Scroll wheel ------------------------------------------------------
    bool onScroll(const ScrollEvent &ev) override
    {
        const float mx = logicalX(ev.pos.getX());
        const float my = logicalY(ev.pos.getY());

        // Menu / splash
        if (fMenu.isSplashOpen() || fMenu.isOpen()) {
            fMenu.onScroll(ev.delta.getY());
            repaint();
            return true;
        }

        int param = fPage.hitTest(mx, my);
        if (param < 0) return false;

        float lo = fParamMeta[param].min;
        float hi = fParamMeta[param].max;
        int   steps = fParamMeta[param].steps;

        float step   = (steps > 0) ? (hi - lo) / (float)steps
                                   : (hi - lo) / CONTINUOUS_STEPS;
        float speed = speedMult(param, ev.mod);
        float newVal = fParamValues[param] + (float)(ev.delta.getY() * speed) * step;
        newVal = std::max(lo, std::min(hi, newVal));
        newVal = snapToSteps(newVal, lo, hi, steps);

        editParameter(param, true);
        setParameterValue(param, newVal);
        editParameter(param, false);
        parameterChanged(param, newVal);
        return true;
    }

    // -- Keyboard ----------------------------------------------------------
    bool onKeyboard(const KeyboardEvent &ev) override
    {
        // -- Splash / menu  -  delegate to UIMenu ---------------------------
        if (fMenu.isSplashOpen() || fMenu.isOpen()) {
            fMenu.onKey(ev.key, ev.press,
                        kKeyUp, kKeyDown, kKeyLeft, kKeyRight,
                        kKeyPageUp, kKeyPageDown);
            if (fMenu.takeNeedsFullRedraw()) {
                requestRedraw(RedrawLevel::Full);
                repaint();
            }
            return true; // always consume  -  prevent host acting on keys
        }

        if (ev.key == 9) {  // Tab  -  cycle page
            if (ev.press) {
                fPageBtnPressed = true;
                requestRedraw(RedrawLevel::Full);
                repaint();
            } else {
                fPageBtnPressed = false;
                fPage.switchPage(fPage.currentPage() == 1 ? 2 : 1);
                requestRedraw(RedrawLevel::Full);
                repaint();
            }
            return true;
        }

        // Track modifier keys live so onMotion can read the current speed.
        // When a modifier changes during a drag, re-anchor (update fDragStartY
        // and fDragStartVal to the current state) so the knob doesn't jump.
        // Handle both L and R variants  -  kKeyShift/kKeyControl are
        // deprecated.
        const bool isShift   = (ev.key == kKeyShiftL   || ev.key == kKeyShiftR);
        const bool isControl = (ev.key == kKeyControlL || ev.key == kKeyControlR);
        if (isShift || isControl) {
            uint newMod = fCurrentMod;
            if (isShift)
                newMod = ev.press ? (newMod |  kModifierShift)
                                  : (newMod & ~kModifierShift);
            else
                newMod = ev.press ? (newMod |  kModifierControl)
                                  : (newMod & ~kModifierControl);

            if (newMod != fCurrentMod && fDragParam >= 0) {
                // Re-anchor: make the current knob position the new drag origin
                // so the speed change takes effect without any jump.
                fDragStartVal = fParamValues[fDragParam];
                fDragStartY   = fDragLastY;
            }
            fCurrentMod = newMod;
            return true;
        }

        if (!ev.press) return false;

        // Esc  -  clear selection, but remember it for next arrow key
        if (ev.key == 27) {
            int prevSel = fPage.selectedParam();
            fPage.clearSelection();
            if (prevSel >= 0) fPage.setDirty(prevSel);
            requestRedraw(RedrawLevel::Display);
            repaint();
            return true;
        }

        // Arrow keys  -  spatial navigation
        if (ev.key == kKeyLeft || ev.key == kKeyRight ||
            ev.key == kKeyUp   || ev.key == kKeyDown) {
            if (fPage.selectedParam() < 0 && fPage.lastSelectedParam() >= 0) {
                // Restore after Esc, no movement
                fPage.restoreSelection();
                fPage.setDirty(fPage.selectedParam());
                requestRedraw(RedrawLevel::Display);
                repaint();
                return true;
            }
            int prevSel = fPage.selectedParam();
            fPage.navigate(ev.key);
            int newSel = fPage.selectedParam();
            if (prevSel >= 0) fPage.setDirty(prevSel);
            if (newSel  >= 0) fPage.setDirty(newSel);
            requestRedraw(RedrawLevel::Display);
            repaint();
            return true;
        }

        // '+' / 'k' / Page Up  -  increase selected parameter
        if (ev.key == '+' || ev.key == 'k' || ev.key == kKeyPageUp) {
            if (fPage.selectedParam() >= 0)
                adjustSelected(1, ev.mod);
            return true;
        }

        // '-' / 'j' / Page Down  -  decrease selected parameter
        if (ev.key == '-' || ev.key == 'j' || ev.key == kKeyPageDown) {
            if (fPage.selectedParam() >= 0)
                adjustSelected(-1, ev.mod);
            return true;
        }

        // 'u' / 'm'  -  toggle the UI menu open/closed
        if (ev.key == 'u' || ev.key == 'm') {
            if (fMenu.isOpen()) {
                fMenu.close();
            } else {
                float menuX = fStrip.geometry().uiBtnX + fStrip.geometry().uiBtnW;
                float menuY = BTN_STRIP_SEP_Y + MENU_EDGE_MARGIN;
                fMenu.open(menuX, menuY, UI_W, UI_H);
            }
            requestRedraw(RedrawLevel::Full);
            repaint();
            return true;
        }

        return false;
    }

    // -- Coordinate helpers ------------------------------------------------
    // Convert a physical window pixel coordinate to a logical UI coordinate.
    // The UI surface occupies UI_W*s x UI_H*s pixels anchored at the top-left
    // of the window; any remaining window area is empty margin.  Dividing by
    // the surface size (not the full window size) gives correct hit-testing
    // when the window aspect ratio doesn't match the UI aspect ratio.
    float logicalX(int px) const { return px / fUIScale; }
    float logicalY(int py) const { return py / fUIScale; }

    // Snap a value to the nearest integer step if the parameter is stepped
    static float snapToSteps(float val, float lo, float hi, int steps)
    {
        if (steps <= 0) return val;
        float norm = (val - lo) / (hi - lo);
        float snapped = std::round(norm * (float)steps) / (float)steps;
        return lo + snapped * (hi - lo);
    }

    // Speed multiplier for modifier keys.
    // Continuous params: Shift = 10x, Ctrl = 0.1x, both or neither = 1x.
    // Integer params (SP_INTS): Shift = 12x for octave jumps,
    // Ctrl/neither = 1x.
    // Enum params (labelled scale points): always 1x, unaffected by modifiers.
    float speedMult(int paramNo, uint mod) const
    {
        const int spId = fParamMeta[paramNo].spId;
        bool shift = (mod & kModifierShift)   != 0;
        bool ctrl  = (mod & kModifierControl) != 0;
        if (spId == SP_INTS) return shift ? 12.0f : 1.0f;
        if (spId != SP_NONE) return 1.0f;  // enum: unaffected
        if (shift && !ctrl) return DRAG_SPEED_FAST;
        if (ctrl  && !shift) return DRAG_SPEED_SLOW;
        return 1.0f;
    }

    // -- Adjust selected parameter by one step ----------------------------
    void adjustSelected(int direction, uint mod = 0)
    {
        const int sel = fPage.selectedParam();
        if (sel < 0) return;
        const ParamMeta &m = fParamMeta[sel];
        float lo    = m.min;
        float hi    = m.max;
        int   steps = m.steps;
        float step  = (steps > 0) ? (hi - lo) / (float)steps
                                  : (hi - lo) / CONTINUOUS_STEPS;
        float speed = speedMult(sel, mod);
        float newVal = fParamValues[sel] + direction * speed * step;
        newVal = std::max(lo, std::min(hi, newVal));
        newVal = snapToSteps(newVal, lo, hi, steps);

        editParameter(sel, true);
        setParameterValue(sel, newVal);
        editParameter(sel, false);
        // selected param value changed -> display panel needs update
        parameterChanged(sel, newVal);
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MiMiUI)
};

// -----------------------------------------------------------------------------
// DPF factory function
// -----------------------------------------------------------------------------

UI *createUI() { return new MiMiUI(); }

END_NAMESPACE_DISTRHO
