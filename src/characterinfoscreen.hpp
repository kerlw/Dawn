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
   along with this program.  If not, see <http://www.gnu.org/licenses/>. **/

#ifndef CHARACTER_INFO_SCREEN_HPP
#define CHARACTER_INFO_SCREEN_HPP

#include "texture.hpp"
#include "framesbase.hpp"

class Player;
class GLFT_Font;

struct sTabs
{
    int posX, posY, width, height;
    CTexture tabimage;
};

class CharacterInfoScreen : public FramesBase
{
private:
	Player* player;
	uint8_t currentTab;
	CTexture textures;
	GLFT_Font* infoFont;
	sTabs tabs[3];


public:
	CharacterInfoScreen( Player* player );
	~CharacterInfoScreen();

	void clicked( int mouseX, int mouseY, uint8_t mouseState );

	void LoadTextures();
	void initFonts();

	void draw( int mouseX, int mouseY );
	void drawExpBar();
	void drawTabs();
};

#endif
