/*
	==============================================================================
	This file is part of the MiMi-d synthesizer.

	Copyright 2024 Ricard Wanderlof

	This file may be licensed under the terms of of the
	GNU General Public License Version 2 (the ``GPL'').

	Software distributed under the License is distributed
	on an ``AS IS'' basis, WITHOUT WARRANTY OF ANY KIND, either
	express or implied. See the GPL for the specific language
	governing rights and limitations.

	You should have received a copy of the GPL along with this
	program. If not, go to http://www.gnu.org/licenses/gpl.html
	or write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
	==============================================================================
 */

#ifndef _DISTRHO_PLUGIN_INFO_H_
#define _DISTRHO_PLUGIN_INFO_H_

#define DISTRHO_PLUGIN_BRAND "Pollux"
#define DISTRHO_PLUGIN_NAME  "MiMi-d"
#define DISTRHO_PLUGIN_URI   "https://butoba.net/homepage/mimid.html"

#define DISTRHO_PLUGIN_NUM_INPUTS   0
#define DISTRHO_PLUGIN_NUM_OUTPUTS  2
#define DISTRHO_PLUGIN_IS_SYNTH 1
#define DISTRHO_PLUGIN_HAS_UI 0
#define DISTRHO_PLUGIN_IS_RT_SAFE   1
#define DISTRHO_PLUGIN_WANT_TIMEPOS   1

// Bring in Parameters enum */
#include "Engine/ParamsEnum.h"

#endif /* _DISTRHO_PLUGIN_INFO_H_ */
