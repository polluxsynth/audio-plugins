/*
	==============================================================================
	This file is part of the MiMi-d synthesizer,
	originally from Obxd synthesizer.

	Copyright © 2013-2014 Filatov Vadim
	Copyright 2023 Ricard Wanderlof

	Contact original author via email :
	justdat_@_e1.ru

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
#pragma once

enum Parameters
{

#define PARAMPOINTS(SPID, ...)
#define PARAMRANGE(SPID, MIN, MAX)
#define PARAMGROUP(PGID, NAME, SYMBOL)
#define PARAM(PARAMNO, PG, SP, NAME, SYMBOL, MIN, MAX, DEFAULT, SETFUNC) PARAMNO,
#define PARAM_NULL(PARAMNO, NAME, SYMBOL) PARAMNO,

// This brings in the parameters as enum members
#include "ParamDefs.h"

	PARAM_COUNT,
};

enum ParameterGroups
{
#define PARAMPOINTS(SPID, ...)
#define PARAMRANGE(SPID, MIN, MAX)
#define PARAMGROUP(PGID, NAME, SYMBOL) PGID,
#define PARAM(PARAMNO, PG, SP, NAME, SYMBOL, MIN, MAX, DEFAULT, SETFUNC)
#define PARAM_NULL(PARAMNO, NAME, SYMBOL)

// This brings in the parameter groups as enum members
#include "ParamDefs.h"

};

enum ScalePoints
{
#define PARAMPOINTS(SPID, ...) SPID,
#define PARAMRANGE(SPID, MIN, MAX) SPID,
#define PARAMGROUP(PGID, NAME, SYMBOL)
#define PARAM(PARAMNO, PG, SP, NAME, SYMBOL, MIN, MAX, DEFAULT, SETFUNC)
#define PARAM_NULL(PARAMNO, NAME, SYMBOL)

// This brings in the scale points as enum members
#include "ParamDefs.h"

	SP_COUNT,
};
