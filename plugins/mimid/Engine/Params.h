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
#include "Voice.h"
#include "ParamsEnum.h"

#define PARAMPOINTS(SPID, ...)
#define PARAMRANGE(SPID, MIN, MAX)
#define PARAMGROUP(PGID, NAME, SYMBOL)

#define PARAM(PARAMNO, NAME, PG, SP, SYMBOL, MIN, MAX, DEFAULT, SETFUNC) \
	values[PARAMNO] = DEFAULT;

class Params
{
public:
	float values[PARAM_COUNT];
	String name;
	Params()
	{
		name = "Default";
		setDefaultValues();
	}
	void setDefaultValues()
	{
// Including "ParamDefs" with PARAM set as above will initalize
// all defined parameters to default values
#include "ParamDefs.h"
	}
	~Params()
	{
	}
	//JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Params)
};
