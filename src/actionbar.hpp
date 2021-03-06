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

#ifndef ACTIONBAR_HPP
#define ACTIONBAR_HPP

#include "tooltip.hpp"
#include "spellbook.hpp"

class Player;

struct sButton
{
  CSpellActionBase* action;
  spellTooltip* tooltip;
  std::string number;
  int posX;
  int posY;
  uint8_t height;
  uint8_t width;
  bool wasPressed;
  int actionSpecificXPos;
  int actionSpecificYPos;
  bool actionReadyToCast;
  bool areaOfEffectOnSpecificLocation;
  SDLKey key;

  sButton( int posX_, int posY_, int width_, int height_, std::string number_, SDLKey key_)
  {
    posX = posX_;
    posY = posY_;
    height = height_;
    width = width_;
    number = number_;
    key	= key_;
    tooltip = NULL;
    action = NULL;
    actionSpecificXPos = 0;
    actionSpecificYPos = 0;
    actionReadyToCast = true;
    areaOfEffectOnSpecificLocation = false;
    wasPressed	= false;
  };
};

class ActionBar
{
public:
  ActionBar( Player* player_ );
  ~ActionBar();

  bool isPreparingAoESpell() const;
  void makeReadyToCast( int x, int y );
  void stopCastingAoE();
  bool isMouseOver( int x, int y );
  bool isButtonUsed( sButton* button ) const;
  bool isSpellUseable( CSpellActionBase* action );
  void draw();
  void drawSpellTooltip( int x, int y );
  void clicked( int clickX, int clickY );
  void executeSpellQueue();
  void handleKeys();
  void dragSpell();
  int8_t getMouseOverButtonId( int x, int y );
  void bindAction ( sButton* button, CSpellActionBase* action );
  void unbindAction ( sButton* button );
  void setSpellQueue( sButton& button, bool actionReadyToCast = true );
  void loadTextures();
  void initFonts();
  std::string getLuaSaveText();
  void bindActionToButtonNr( int buttonNr, CSpellActionBase* action );
  void clear();

private:
  Player* player;
  int posX;
  int posY;
  int width;
  int height;
  std::vector <sButton> button;
  CTexture textures;
  GLFT_Font* shortcutFont;
  GLFT_Font* cooldownFont;
  sButton* spellQueue;
  std::vector<std::pair<CSpellActionBase*, uint32_t> > cooldownSpells;
  bool preparingAoESpell;
  int cursorRadius;
};

#endif
