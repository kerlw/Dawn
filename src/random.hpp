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

#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <cstring>

namespace RNG // RandomNumberGenerator
{
  void initRNG( unsigned int seed );
  size_t randomSizeT( size_t min, size_t max );
  int randomInt( int min, int max );
  double randomDouble( double min, double max );
}

#endif
