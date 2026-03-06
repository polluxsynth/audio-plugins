// -----------------------------------------------------------------------------
// UISettings  -  runtime UI configuration
//
// Shared between MiMiUI.cpp (file I/O, ownership) and UIMenu.h (display).
// -----------------------------------------------------------------------------

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

// Three-state knob value display
enum ShowValue { SHOW_NONE = 0, SHOW_STEPPED = 1, SHOW_ALL = 2 };

struct UISettings {
    double    uiScale   = 1.0;
    bool      showIndex = true;
    bool      showArc   = true;
    ShowValue showValue = SHOW_ALL;
    bool      snapScale = true;
    bool      ghostMode = false;
};
