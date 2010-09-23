/**
    Copyright (C) 2009,2010  Dawn - 2D roleplaying game

    This file is a part of the dawn-rpg project <http://sourceforge.net/projects/dawn-rpg/>.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. **/

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>

namespace Configuration
{
	extern bool fullscreenenabled;
	extern bool soundenabled;
	extern int screenWidth;
	extern int screenHeight;
	extern int bpp;
	
	extern std::string logfile; // The logfile
	extern bool debug_stdout; // Write messages to stdout
	extern bool debug_fileout; // Write messages to a file
	extern bool show_info_messages; // Show or hide debug info
	extern bool show_warn_messages; // show or hide warnings
}

#endif // CONFIGURATION_H