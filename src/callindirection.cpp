/* Copyright (C) 2009,2010,2011,2012  Dawn - 2D roleplaying game

   This file is a part of the dawn-rpg project <https://github.com/frusen/Dawn>.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "callindirection.hpp"
#include "luafunctions.hpp"
#include "globals.hpp"
#include "zone.hpp"
#include <sstream>

LuaCallIndirection::LuaCallIndirection()
{
	luaText = "";
}

void LuaCallIndirection::call()
{
	if ( luaText != "" ) {
		LuaFunctions::executeLuaScript( luaText );
	}
}

std::string LuaCallIndirection::getLuaSaveText() const
{
	std::ostringstream oss;
	std::string objectName = "curEventHandler";
	oss << "local " << objectName << " = DawnInterface.createEventHandler();" << std::endl;
	oss << objectName << ":setExecuteText( [[" << luaText << "]] );" << std::endl;

	return oss.str();
}

void LuaCallIndirection::setExecuteText( std::string text )
{
	this->luaText = text;
}

namespace DawnInterface
{
	LuaCallIndirection* createEventHandler()
	{
		LuaCallIndirection *newEventHandler = new LuaCallIndirection();
		Globals::getCurrentZone()->addEventHandler( newEventHandler );
		return newEventHandler;
	}

}

