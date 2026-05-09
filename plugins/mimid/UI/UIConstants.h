/*
 * UIConstants.h  -  All tweakable appearance constants for MiMi-d UI
 *
 * Every magic number that affects appearance lives here.
 * Geometry constants correspond directly to the preview sliders.
 * Colour constants are RGBA in [0.f.1f] range, used with Cairo's
 * cairo_set_source_rgba().
 *
 * After changing any value here, recompile  -  no other files need editing.
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

// ===========================================================================
// GEOMETRY  (all in logical units; the UI is scaled to fit the window)
// ===========================================================================

// -- Knob ------------------------------------------------------------------
// Knob body radius (logical units)
static constexpr float KNOB_R            = 20.0f;

// Arc track: track arc is drawn at trackR = KNOB_R + TRACK_W * TRACK_R_MULT
// stroke width of arc
static constexpr float TRACK_W           =  2.5f;
// track radius multiplier
static constexpr float TRACK_R_MULT      =  2.6f;

// Derived: trackR = KNOB_R + TRACK_W * TRACK_R_MULT
// (computed at runtime in G(), not a constexpr because it depends on KNOB_R)

// Extra gap between knob centres beyond 2*trackR
static constexpr float KNOB_PITCH_EXTRA  = 22.0f;

// Indicator line: inner and outer as fraction of KNOB_R
static constexpr float IND_R_INNER       = 0.52f;
static constexpr float IND_R_OUTER       = 0.82f;
// line width as fraction of KNOB_R
static constexpr float IND_WIDTH_RATIO   = 0.09f;
// minimum line width in logical units
static constexpr float IND_WIDTH_MIN     = 1.8f;

// Knob face gradient: flat zone as fraction of KNOB_R (0=all bevel, 1=all flat)
static constexpr float KNOB_FLAT_ZONE    = 0.85f;
// How much lighter the flat centre is vs base colour (0..1 additive)
static constexpr float KNOB_CENTRE_LIFT  = 0.14f;
// How much darker the rim bevel is vs base colour (0..1 subtractive)
static constexpr float KNOB_RIM_DARKEN   = 0.50f;
// Top-edge specular: opacity at top, fading to 0 at this fraction down
static constexpr float KNOB_SPEC_OPACITY = 0.10f;
// fraction of radius
static constexpr float KNOB_SPEC_FADE_Y  = 0.45f;
// specular arc radius as fraction of knob radius
static constexpr float KNOB_SPEC_RADIUS  = 0.98f;

// Drop shadow: filled circle offset behind the knob face,
// same technique as the button shadow (BTN_SHADOW_OX/OY).
// Only a crescent at the lower-right edge stays visible.
// X/Y offsets in pixels; DEPTH is extra radius beyond KNOB_R.
static constexpr float KNOB_SHADOW_OX    =  1.0f;
static constexpr float KNOB_SHADOW_OY    =  3.0f;
static constexpr float KNOB_SHADOW_DEPTH =  1.5f;
static constexpr float KNOB_SHADOW_ALPHA =  0.30f;

// rim stroke width
static constexpr float KNOB_RIM_W          =  3.0f;

// peak alpha of selection glow
static constexpr float KNOB_GLOW_PEAK_ALPHA = 0.95f;
// normalised gradient position of peak alpha in selection glow
// (0 = knob edge, 1 = track outer edge)
static constexpr float KNOB_GLOW_PEAK_POS  = 0.35f;

// -- Knob dome gloss -------------------------------------------------------
// Radial white overlay creating a centre-bright gloss effect.
// alpha at centre
static constexpr float KNOB_DOME_ALPHA_CTR   =  0.18f;
// normalised radius of mid stop
static constexpr float KNOB_DOME_MID_POS     =  0.55f;
// alpha at mid stop
static constexpr float KNOB_DOME_ALPHA_MID   =  0.04f;

// Extra margin added to trackR for mouse hit testing  -  makes knobs
// slightly easier to click than their visual arc radius alone.
static constexpr float KNOB_HIT_MARGIN     =  4.0f;

// Vertical drag distance (logical pixels) required to sweep a continuous
// parameter across its full range at normal speed (speed multiplier = 1.0).
// Shift makes this effectively 10x shorter; Ctrl makes it 10x longer.
static constexpr float DRAG_FULL_RANGE_PX  = 200.0f;
// Shift multiplier
static constexpr float DRAG_SPEED_FAST     =  10.0f;
// Ctrl multiplier
static constexpr float DRAG_SPEED_SLOW     =   0.1f;

// -- Arc track ticks -------------------------------------------------------
// All lengths expressed as multiples of TRACK_W
// end tick inner extent
static constexpr float TICK_END_INNER    =  1.8f;
// end tick outer extent (kept inside redraw bounding box)
static constexpr float TICK_END_OUTER    =  1.0f;
static constexpr float TICK_STEP_INNER   =  1.5f;
static constexpr float TICK_STEP_OUTER   =  1.5f;
// stroke width
static constexpr float TICK_END_WIDTH    =  1.6f;
static constexpr float TICK_STEP_WIDTH   =  1.0f;
static constexpr float TICK_ACTIVE_WIDTH =  1.8f;

// -- Arc angles ------------------------------------------------------------
// 270 deg range, gap at bottom (90 deg gap centred at 270 deg/6-o'clock)
// Start = 135 deg (lower-left), clockwise to 45 deg (lower-right)
static constexpr float ARC_START_DEG    = 135.0f;
static constexpr float ARC_RANGE_DEG   = 270.0f;

// -- Labels ----------------------------------------------------------------
// Font registration names  -  must match the names passed to
// createFontFromMemory()
static constexpr const char *FONT_NAME_REGULAR = "regular";
static constexpr const char *FONT_NAME_BOLD    = "bold";

// Font size as fraction of KNOB_R (applies to value text, param name,
// group title)
// font size as fraction of knob radius
static constexpr float FONT_SIZE_RATIO  = 0.50f;
// minimum font size in logical units; used as a legibility guard if KNOB_R
// is set to a small value because we want small knobs.
static constexpr float FONT_SIZE_MIN    = 10.0f;
// label/title font relative to value font
static constexpr float LABEL_FONT_SCALE = 1.20f;

// Distance from knob centre to top of name label (logical units beyond trackR).
// Negative value compensates for the renderer reporting a larger
// ascent bounding box than the actual text ink, which would otherwise
// leave excess space above the text.
static constexpr float NAME_DY_OFFSET   = -3.0f;
// extra height above labelFontSize for name bounding box
static constexpr float NAME_H_PAD       = 7.0f;

// -- Frame -----------------------------------------------------------------
// Asymmetric vertical padding inside frame
// above knob centre
static constexpr float FRAME_VPAD_TOP   = 12.0f;
// below name label
static constexpr float FRAME_VPAD_BOT   =  0.0f;

// Gaps between adjacent frames
// horizontal gap
static constexpr float FRAME_GAP_H      =  6.0f;
// vertical gap
static constexpr float FRAME_GAP_V      = 10.0f;

// Visual
// rounded corner radius
static constexpr float FRAME_CORNER_R   =  7.0f;
// stroke width
static constexpr float FRAME_LINE_W     =  1.4f;
// title text x offset from frame left
static constexpr float FRAME_TITLE_X    =  6.0f;
// padding each side of title text in gap
static constexpr float FRAME_TITLE_PAD  =  4.0f;

// -- Buttons ---------------------------------------------------------------
// corner radius as fraction of height
static constexpr float BTN_CORNER_RATIO = 0.28f;
// shadow opacity
static constexpr float BTN_SHADOW_ALPHA = 0.40f;
static constexpr float BTN_SHADOW_OX    = 1.0f;
static constexpr float BTN_SHADOW_OY    = 3.0f;
// font size as fraction of button height
static constexpr float BTN_LABEL_H_RATIO= 0.81f;
// button border stroke width
static constexpr float BTN_BORDER_W     = 1.5f;

// -- Parameter display panel -----------------------------------------------
// rounded corner radius
static constexpr float DISP_CORNER_R   =  3.0f;
// border stroke opacity
static constexpr float DISP_BORDER_A   =  0.50f;
// border stroke width
static constexpr float DISP_BORDER_STROKE_W =  1.0f;
// inner shadow opacity
static constexpr float DISP_SHADOW_A   =  0.40f;
// inner shadow stroke width
static constexpr float DISP_SHADOW_STROKE_W =  1.0f;
// inner shadow y offset from top
static constexpr float DISP_SHADOW_Y   =  1.2f;
// text left margin from panel left edge
static constexpr float DISP_TEXT_X     =  8.0f;
// background fill opacity over COL_BACKGROUND
static constexpr float DISP_FILL_ALPHA =  0.40f;
// inset from panel edge before clearing/clipping text area
static constexpr float DISP_INSET      =  2.0f;
// right padding kept clear of value text (prevents overdraw at right edge)
static constexpr float DISP_VALUE_RPAD =  4.0f;

// -- Button strip (page selector bar at top) -------------------------------
// height of button strip
static constexpr float BTN_STRIP_H        = 48.0f;
// horizontal padding each side of button label
static constexpr float PAGE_BTN_HPAD      = 10.0f;
static constexpr float PAGE_BTN_H         = 28.0f;
static constexpr float PAGE_BTN_Y         = (BTN_STRIP_H - PAGE_BTN_H) / 2.0f;
// y of separator line
static constexpr float BTN_STRIP_SEP_Y    = BTN_STRIP_H;

// Left/top margin from strip edge to first knob grid origin
static constexpr float GRID_ORIGIN_MARGIN = 8.0f;

// Number of scroll/step increments for a continuous parameter across
// its full range
static constexpr float CONTINUOUS_STEPS    = 100.0f;

// Minimum interval between value-update redraws (milliseconds).
// Full and Display redraws always execute immediately.
static constexpr int   UPDATE_DRAW_INTERVAL_MS = 0;

// title font size as fraction of BTN_STRIP_H
static constexpr float STRIP_TITLE_FONT_RATIO = 0.75f;
// left margin for plugin name title
static constexpr float STRIP_TITLE_X      =  8.0f;
// gap between VBreak line and adjacent button edge
static constexpr float STRIP_BTN_GAP      =  2.0f;
// maximum width of parameter display panel
static constexpr float STRIP_DISP_MAX_W   = 350.0f;
// minimum display panel width; below this it is not drawn
static constexpr float STRIP_DISP_MIN_W   =  40.0f;

// Engraved separator line colours  -  dark line + 1px offset light line
// dark line opacity
static constexpr float STRIP_SEP_DARK_A   = 0.35f;
// light line opacity (highlight)
static constexpr float STRIP_SEP_LIGHT_A  = 0.10f;
// pixel offset of the light highlight line in engraved-line pairs
static constexpr float ENGRAVE_OFFSET     =  1.0f;

// -- Derived constexpr geometry (mirrors UIGeometry runtime values) ---------
// These duplicate the UIGeometry formulas so that UI_W/UI_H can be computed
// at compile time for use in the UI() constructor.

static constexpr float _TRACK_R   = KNOB_R + TRACK_W * TRACK_R_MULT;
static constexpr float _KPITCH    = 2.0f * _TRACK_R + KNOB_PITCH_EXTRA;
static constexpr float _FONT_SIZE       = (KNOB_R * FONT_SIZE_RATIO < FONT_SIZE_MIN)
                                   ? FONT_SIZE_MIN : KNOB_R * FONT_SIZE_RATIO;
static constexpr float _LABEL_FONT_SIZE = _FONT_SIZE * LABEL_FONT_SCALE;
static constexpr float _NAME_H    = _LABEL_FONT_SIZE + NAME_H_PAD;
static constexpr float _NAME_DY   = _TRACK_R + NAME_DY_OFFSET;
static constexpr float _TOP_HALF  = _TRACK_R + FRAME_VPAD_TOP;
static constexpr float _BOT_HALF  = _NAME_DY + _NAME_H + FRAME_VPAD_BOT;
static constexpr float _ROW_PITCH = _TOP_HALF + _BOT_HALF + FRAME_GAP_V;

// UI_SIZE(COLS, ROWS)  -  declare UI_W and UI_H from layout size in knob units.
// COLS = number of knob columns (0-based max col + 1)
// ROWS = number of knob rows    (0-based max row + 1)
#define UI_SIZE(COLS, ROWS) \
    static constexpr uint UI_W = (uint)(_KPITCH / 2.0f + GRID_ORIGIN_MARGIN \
                                      + ((COLS) - 1) * _KPITCH \
                                      + _KPITCH / 2.0f + GRID_ORIGIN_MARGIN); \
    static constexpr uint UI_H = (uint)(BTN_STRIP_H + GRID_ORIGIN_MARGIN + _TOP_HALF \
                                      + ((ROWS) - 1) * _ROW_PITCH \
                                      + _BOT_HALF + GRID_ORIGIN_MARGIN);

// ===========================================================================
// COLOURS  (R, G, B, A  -  all in [0..1])
// ===========================================================================

struct RGBA { float r, g, b, a; };

// -- Background ------------------------------------------------------------
// RAL 5015 Sky Blue
static constexpr RGBA COL_BACKGROUND       = { 0.000f, 0.475f, 0.671f, 1.0f };

// -- Knob ------------------------------------------------------------------
// #404040
static constexpr RGBA COL_KNOB_BASE        = { 0.302f, 0.301f, 0.251f, 1.0f };
// selection halo  -  page 1 - warm yellow
static constexpr RGBA COL_KNOB_GLOW        = { 1.00f,  0.80f,  0.10f,  1.0f  };
// selection halo  -  page 2 - orange
static constexpr RGBA COL_KNOB_GLOW_P2     = { 1.00f,  0.55f,  0.20f,  1.0f  };
static constexpr RGBA COL_KNOB_RIM         = { 0.000f, 0.000f, 0.000f, 0.60f };

// -- Arc track -------------------------------------------------------------
static constexpr RGBA COL_TRACK_BG         = { 0.000f, 0.000f, 0.000f, 0.38f };
static constexpr RGBA COL_TRACK_FILL       = { 1.000f, 1.000f, 1.000f, 0.85f };

// -- Ticks -----------------------------------------------------------------
static constexpr RGBA COL_TICK_END         = { 1.000f, 1.000f, 1.000f, 0.72f };
static constexpr RGBA COL_TICK_STEP        = { 1.000f, 1.000f, 1.000f, 0.55f };
static constexpr RGBA COL_TICK_ACTIVE      = { 1.000f, 1.000f, 1.000f, 0.90f };

// -- Indicator -------------------------------------------------------------
static constexpr RGBA COL_INDICATOR        = { 1.000f, 1.000f, 1.000f, 0.94f };

// -- Labels ----------------------------------------------------------------
static constexpr RGBA COL_VALUE_TEXT       = { 1.000f, 1.000f, 1.000f, 0.95f };
static constexpr RGBA COL_NAME_TEXT        = { 1.000f, 1.000f, 1.000f, 0.95f };
static constexpr RGBA COL_FRAME_LINE       = { 1.000f, 1.000f, 1.000f, 0.70f };
static constexpr RGBA COL_FRAME_TITLE      = { 1.000f, 1.000f, 1.000f, 0.95f };
static constexpr RGBA COL_KNOB_LABEL       = { 1.000f, 1.000f, 1.000f, 0.95f };

// -- Buttons ---------------------------------------------------------------
// Off state gradient stops  -  matches knob face centre colour for consistency
// Knob centre = lighten(COL_KNOB_BASE, KNOB_CENTRE_LIFT) = ~0.391 grey
// lighter top edge
static constexpr RGBA COL_BTN_OFF_TOP      = { 0.490f, 0.490f, 0.440f, 1.0f };
// midpoint between top and bot (3-stop gradient)
static constexpr RGBA COL_BTN_OFF_MID      = { 0.420f, 0.420f, 0.370f, 1.0f };
// darker bottom
static constexpr RGBA COL_BTN_OFF_BOT      = { 0.350f, 0.350f, 0.300f, 1.0f };
static constexpr RGBA COL_BTN_OFF_BORDER   = { 0.000f, 0.000f, 0.000f, 0.55f };
// matches all panel text
static constexpr RGBA COL_BTN_OFF_LABEL    = { 1.000f, 1.000f, 1.000f, 1.00f };

// Pressed state  -  same colour as off, press indicated by offset and no shadow
static constexpr RGBA COL_BTN_PRS_TOP      = { 0.440f, 0.440f, 0.440f, 1.0f };
// midpoint between top and bot (3-stop gradient)
static constexpr RGBA COL_BTN_PRS_MID      = { 0.370f, 0.370f, 0.370f, 1.0f };
static constexpr RGBA COL_BTN_PRS_BOT      = { 0.300f, 0.300f, 0.300f, 1.0f };
static constexpr RGBA COL_BTN_PRS_BORDER   = { 0.000f, 0.000f, 0.000f, 0.55f };
static constexpr RGBA COL_BTN_PRS_LABEL    = { 1.000f, 1.000f, 1.000f, 1.00f };

// Specular overlay
static constexpr RGBA COL_BTN_SPEC_OFF     = { 1.000f, 1.000f, 1.000f, 0.09f };
static constexpr RGBA COL_BTN_SPEC_PRS     = { 1.000f, 1.000f, 1.000f, 0.03f };

// Amount button shifts down+right when pressed (logical units)
static constexpr float BTN_PRESS_OFFSET   = 2.0f;

// 3-stop gradient: normalised position of the middle stop
static constexpr float BTN_GRAD_MID       = 0.55f;

// -- Menu ------------------------------------------------------------------
static constexpr RGBA COL_MENU_BG          = { 0.491f, 0.491f, 0.491f, 0.97f };
static constexpr RGBA COL_MENU_BG_HOVER    = { COL_BACKGROUND.r, COL_BACKGROUND.g, COL_BACKGROUND.b, 0.55f };
static constexpr RGBA COL_MENU_BORDER      = { 0.000f, 0.000f, 0.000f, 0.70f };
// same as items, bold
static constexpr RGBA COL_MENU_HEADER      = { 1.000f, 1.000f, 1.000f, 0.95f };
static constexpr RGBA COL_MENU_ITEM        = { 1.000f, 1.000f, 1.000f, 0.95f };
// "(Restart to take effect)"
static constexpr RGBA COL_MENU_INFO        = { 1.000f, 1.000f, 1.000f, 0.65f };
static constexpr RGBA COL_MENU_SEP         = { 1.000f, 1.000f, 1.000f, 0.15f };
// reverse-video value field
static constexpr RGBA COL_MENU_ARMED_BG    = { 1.000f, 1.000f, 1.000f, 0.90f };
// dark text on light bg
static constexpr RGBA COL_MENU_ARMED_TEXT  = { 0.150f, 0.150f, 0.150f, 1.00f };
// menu item font size in logical units
static constexpr float MENU_FONT_SIZE     = 12.0f;
// smaller font for hint/info text (e.g. splash "click to close")
static constexpr float MENU_HINT_SIZE     = 11.0f;
// horizontal text padding
static constexpr float MENU_PAD_X        = 10.0f;
// top/bottom outer padding
static constexpr float MENU_PAD_Y        =  6.0f;
// height per item row
static constexpr float MENU_ITEM_H       = 18.0f;
// height of separator row
static constexpr float MENU_SEP_H        =  9.0f;
// minimum menu width
static constexpr float MENU_MIN_W        = 220.0f;
static constexpr float MENU_CORNER_R     =  4.0f;
// min gap between menu and canvas edge
static constexpr float MENU_EDGE_MARGIN  =  4.0f;
// border stroke width
static constexpr float MENU_BORDER_W     =  1.0f;
// hover highlight inset from left/right edge
static constexpr float MENU_HOVER_INSET  =  1.0f;

// Indicator tick geometry
// tick glyph size (half-extent)
static constexpr float MENU_TICK_SIZE    =  4.5f;
// tick: horizontal fraction from left corner to bottom vertex
static constexpr float MENU_TICK_ARM     =  0.4f;
// tick: vertical fraction down to bottom vertex; also horizontal fraction
// to top vertex
static constexpr float MENU_TICK_BASE    =  0.6f;
// tick: vertical fraction up to top vertex
static constexpr float MENU_TICK_TOP     =  0.7f;
// distance from right edge to tick anchor
static constexpr float MENU_TICK_OFFSET  =  5.0f;
// tick stroke width
static constexpr float MENU_TICK_W       =  1.5f;
// double-tick spacing multiplier (x MENU_TICK_SIZE)
static constexpr float MENU_TICK_SPACING =  2.2f;

// Armed (edit) value field
// horizontal padding inside reverse-video pill
static constexpr float MENU_ARMED_PAD   =  3.0f;

// Numeric item drag sensitivity: pixels of drag -> value delta
// full drag range in pixels for MENU_DRAG_UNITS units
static constexpr float MENU_DRAG_RANGE  = 200.0f;
// value units spanned by MENU_DRAG_RANGE
static constexpr float MENU_DRAG_UNITS  =   1.0f;
// value step size (2 decimal places)
static constexpr float MENU_STEP        =   0.01f;

// UIScale valid range
static constexpr float UI_SCALE_MIN     =  0.5f;
static constexpr float UI_SCALE_MAX     =  2.0f;

// -- Symbols ----------------------------------------------------------------
// Symbol type identifiers for UILAYOUT_PARAM_POINTS
enum Symbol {
    SYM_NONE   = 0,
    SYM_SAW    = 1, // ramp up, vertical drop
    SYM_RSAW   = 2, // vertical rise, ramp down
    SYM_TRI    = 3, // ramp up then down
    SYM_SQUARE = 4, // 50% duty cycle square
    SYM_PULSE  = 5, // narrow high, wide low
    SYM_RPULSE = 6, // wide low, narrow high
    SYM_INF    = 7, // infinity sign (two arcs + crossed lines)
};

// Gap between outer edge of track arc and inner edge of the nearest
// symbol stroke, in logical units.  Increase to push symbols further out.
static constexpr float SYM_DIST =  1.8f;
// Scaling unit for symbol geometry, in logical units.  All symbol
// dimensions (HW, HH, PW below) are expressed as fractions of this value.
static constexpr float SYM_SIZE = 10.0f;
// Stroke width for symbol lines, in logical units.
static constexpr float SYM_LW   =  1.2f;
// Half-width of the symbol as a fraction of SYM_SIZE.
// The full symbol width is 2 * HW * SYM_SIZE.
static constexpr float SYM_HW   =  0.25f;
// Half-height of the symbol as a fraction of SYM_SIZE.
// The full symbol height is 2 * HH * SYM_SIZE.
static constexpr float SYM_HH   =  0.220f;
// Width of the narrow pulse section in pulse/reverse-pulse symbols,
// as a fraction of SYM_SIZE.
static constexpr float SYM_PW   =  0.14f;
// Infinity symbol (SYM_INF): two 270-degree arcs joined by crossed lines.
// Radius of each lobe arc, as a fraction of SYM_SIZE.
static constexpr float SYM_INF_R  = 0.20f;
// Horizontal offset of each lobe centre from the symbol centre, as a
// fraction of SYM_SIZE.  Must be less than SYM_INF_R * cos(45deg) so that
// the open arc endpoints cross past the centre line and the joining lines
// actually intersect at (cx, cy).
static constexpr float SYM_INF_OX = 0.30f;
// Downward nudge of imaginary arc that the symbols are on.
// There's usually a bit of extra space above the knob for the symbols,
// but not so much below, so nudging them all down can give a more
// balanced visual appearance.
static constexpr float SYM_Y_NUDGE = 1.0f;

// Splash screen
static constexpr float SPLASH_W           = 560.0f;
static constexpr float SPLASH_H           = 288.0f;
// background dimmer opacity
static constexpr float SPLASH_DIM_ALPHA   =  0.22f;
// plugin name text y (relative to splash top)
static constexpr float SPLASH_TITLE_Y     =  38.0f;
// description text y
static constexpr float SPLASH_DESC_Y      =  62.0f;
// first separator line y
static constexpr float SPLASH_SEP_Y       =  98.0f;
// first splash text line y
static constexpr float SPLASH_TEXT_Y      = 114.0f;
// vertical step between text lines
static constexpr float SPLASH_TEXT_LINE_H =  18.0f;
// gap from last text line to second separator
static constexpr float SPLASH_SEP2_PAD    =  00.0f;
// gap from second separator to close hint
static constexpr float SPLASH_CLOSE_PAD   =  16.0f;
