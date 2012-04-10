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

#ifndef ELEMENTS_HPP
#define ELEMENTS_HPP

#include <string>

namespace ElementType
{
  enum ElementType
  {
    Fire,
    Water,
    Air,
    Earth,
    Light,
    Dark,
    Count
  };

  /* Returns a string representation of the elementType.
     Example: Fire -> "Fire" */
  std::string getElementName( ElementType::ElementType elementType );

  /* Returns a string representation of the elementType in lowercase letters.
     Example: Fire -> "fire" */
  std::string getElementNameLowercase( ElementType::ElementType elementType );
}

#endif
