/**
    Copyright (C) 2009  Dawn - 2D roleplaying game

    This file is a part of the dawn-rpg project.

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

#include "InventoryScreen.h"
#include "CDrawingHelpers.h"

#include "inventory.h"
#include "CCharacter.h"
#include "item.h"

#include <cassert>

namespace dawn_configuration
{
	extern int screenWidth;
}

extern std::vector<Item*> groundItems;
extern std::vector<std::pair<int,int> > groundPositions;

InventoryScreen::InventoryScreen( Player *player_ )
	:	player( player_ ),
		posX(0),
		posY(100),
		width(350),
		height(400),
		visible(false),
		floatingSelection( NULL )
{};

InventoryScreen::~InventoryScreen()
{

}

void InventoryScreen::LoadTextures()
{
	textures.texture.reserve(2);
	textures.LoadIMG("data/interface/inventory_base.tga",0);
	textures.LoadIMG("data/white2x2pixel.tga",1);
	posX = dawn_configuration::screenWidth - textures.texture[0].width - 50;
}

void InventoryScreen::dropItemOnGround( InventoryItem *inventoryItem )
{
	groundItems.push_back( inventoryItem->getItem() );
	groundPositions.push_back( std::pair<int,int>( player->getXPos(), player->getYPos() ) );
}

void InventoryScreen::clicked( int clickX, int clickY )
{
	size_t backpackOffsetX = 96;
	size_t backpackOffsetY = 112;

	Inventory *inventory = player->getInventory();

	if ( ! isOnThisScreen(clickX,clickY) || ! isVisible() ) {
		// clicked outside inventory window
		if ( floatingSelection != NULL ) {
			// drop item...
			dropItemOnGround( floatingSelection );
			if ( inventory->containsItem( floatingSelection ) ) {
				inventory->removeItem( floatingSelection );
			}
			delete floatingSelection;
			floatingSelection = NULL;

			return;
		}
	}

	// calculate field index under mouse
	if ( clickX < posX + backpackOffsetX
	     || clickY < backpackOffsetY
	     || clickX > posX + backpackOffsetX + 32 * 10 + (10-1)*3
	     || clickY > backpackOffsetY + 32 * 4 + (4-1)*3 ) {
		return;
	}

	int fieldIndexX = ( clickX - (posX + backpackOffsetX) ) / (32+3);
	int fieldIndexY = ( clickY - backpackOffsetY ) / (32+3);

	if ( floatingSelection != NULL ) {
		if ( inventory->hasSufficientSpaceAt( fieldIndexX, fieldIndexY, floatingSelection->getSizeX(), floatingSelection->getSizeY() ) ) {
			inventory->insertItemAt( floatingSelection, fieldIndexX, fieldIndexY );
			floatingSelection = NULL;
		}
	}
	else if ( ! inventory->isPositionFree( fieldIndexX, fieldIndexY ) ) {
		floatingSelection = inventory->getItemAt( fieldIndexX, fieldIndexY );
		inventory->removeItem( floatingSelection );
	}
}

bool InventoryScreen::isVisible() const
{
    return visible;
}

void InventoryScreen::setVisible(bool newVisible)
{
    visible = newVisible;
}

void InventoryScreen::drawBackpack()
{
	Inventory *inventory = player->getInventory();
	std::vector<InventoryItem*> items = inventory->getBackpackItems();

	size_t numItems = items.size();
	for ( size_t curItemNr=0; curItemNr<numItems; ++curItemNr ) {
		InventoryItem *curInvItem = items[ curItemNr ];
		Item *curItem = curInvItem->getItem();
		CTexture *symbolTexture = curItem->getSymbolTexture();

		size_t backpackOffsetX = 96;
		size_t backpackOffsetY = 112;

		size_t invPosX = curInvItem->getInventoryPosX();
		size_t invPosY = curInvItem->getInventoryPosY();
		size_t sizeX = curItem->getSizeX();
		size_t sizeY = curItem->getSizeY();

		DrawingHelpers::mapTextureToRect( symbolTexture->texture[0].texture,
		                                  world_x + posX + backpackOffsetX + invPosX * 32 + invPosX * 3,
		                                  32 * sizeX + (sizeX-1)*3,
		                                  world_y + backpackOffsetY + invPosY * 32 + invPosY * 3,
		                                  32 * sizeY + (sizeY-1)*3);
	}
}

void InventoryScreen::drawFloatingSelection( int x, int y )
{
	// draw floating selection
	if ( floatingSelection != NULL ) {
		Item *floatingItem = floatingSelection->getItem();
		size_t sizeX = floatingItem->getSizeX();
		size_t sizeY = floatingItem->getSizeY();

		DrawingHelpers::mapTextureToRect( floatingItem->getSymbolTexture()->texture[0].texture,
		                                  x, 32 * sizeX + (sizeX-1)*3,
		                                  y-20, 32 * sizeY + (sizeY-1)*3);
	}
}

void InventoryScreen::drawItemPlacement( int x, int y )
{
	if ( isOnBackpackScreen(x,y) && isVisible() && floatingSelection != NULL )
	{
	    Item *floatingItem = floatingSelection->getItem();
	    Inventory *inventory = player->getInventory();
	    GLfloat shade[4] = { 0.0f, 0.0f, 0.0f, 0.3f };
	    size_t backpackOffsetX = 96;
        size_t backpackOffsetY = 112;
        size_t sizeX = floatingItem->getSizeX();
		size_t sizeY = floatingItem->getSizeY();

        // calculate which backpack-slot we are looking at.
		int fieldIndexX = ( x - (posX + backpackOffsetX) ) / (32+3);
        int fieldIndexY = ( y - backpackOffsetY ) / (32+3);

        // set the shade-color depending on if the item fits or not.
        if ( inventory->hasSufficientSpaceAt( fieldIndexX, fieldIndexY, sizeX, sizeY ) )
        {
            shade[1] = 1.0f; // green color
        } else {
            shade[0] = 1.0f; // red color
        }

        // calculate the size of the shade, if too big, we resize it.
        int shadePosX = world_x + posX + backpackOffsetX + fieldIndexX * 32 + fieldIndexX * 3;
        int shadePosY = world_y + backpackOffsetY-1 + fieldIndexY * 32 + fieldIndexY * 3;
        int shadeWidth = 32 * sizeX + (sizeX-1)*3;
        int shadeHeight = 32 * sizeY + (sizeY-1)*3;

        if ( sizeY + fieldIndexY > 4 )
        {
            shadeHeight = 32 * ( sizeY - (( sizeY + fieldIndexY ) - 4 )) + ( sizeY - (( sizeY + fieldIndexY ) - 3 ))*3;
        }

        if ( sizeX + fieldIndexX > 10 )
        {
            shadeWidth = 32 * ( sizeX - (( sizeX + fieldIndexX ) - 10 )) + ( sizeX - (( sizeX + fieldIndexX ) - 9 ))*3;
        }

        glColor4fv(shade);
        DrawingHelpers::mapTextureToRect( textures.texture[1].texture,
                                          shadePosX,
                                          shadeWidth,
                                          shadePosY,
                                          shadeHeight);
        glColor4f(1.0f,1.0f,1.0f,1.0f);
	}
}


void InventoryScreen::draw()
{
    DrawingHelpers::mapTextureToRect( textures.texture[0].texture,
                                      world_x + posX, textures.texture[0].width, world_y + posY, textures.texture[0].height);

	drawBackpack();
}

bool InventoryScreen::isOnThisScreen( int x, int y ) const
{
	if ( x < posX
	     || y < posY
	     || x > posX + textures.texture[0].width
	     || y > posY + textures.texture[0].height ) {
		return false;
	}
	return true;
}

bool InventoryScreen::isOnBackpackScreen( int x, int y ) const
{
    int slotPosX = posX + 96;
    int slotPosY = posY + 11;
    if ( x < slotPosX
	     || y < slotPosY
	     || x > slotPosX + 347
	     || y > slotPosY+ 137 ) {
		return false;
	}
	return true;
}

bool InventoryScreen::hasFloatingSelection() const
{
	return floatingSelection != NULL;
}

void InventoryScreen::selectFloating( InventoryItem *item )
{
	assert( floatingSelection == NULL );
	floatingSelection = item;
}

