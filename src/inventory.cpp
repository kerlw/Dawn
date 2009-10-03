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

#include "inventory.h"

#include "item.h"
#include <cassert>


InventoryItem::InventoryItem( Item *item_, size_t inventoryPosX_, size_t inventoryPosY_ )
	:	item( item_ ),
		inventoryPosX( inventoryPosX_ ),
		inventoryPosY( inventoryPosY_ )
{
}

size_t InventoryItem::getInventoryPosX() const
{
	return inventoryPosX;
}

size_t InventoryItem::getInventoryPosY() const
{
	return inventoryPosY;
}

void InventoryItem::setInventoryPos( size_t inventoryPosX_, size_t inventoryPosY_ )
{
	inventoryPosX = inventoryPosX_;
	inventoryPosY = inventoryPosY_;
}

size_t InventoryItem::getSizeX() const
{
	return item->getSizeX();
}

size_t InventoryItem::getSizeY() const
{
	return item->getSizeY();
}

Item* InventoryItem::getItem() const
{
	return item;
}


/// Class Inventory

Inventory::Inventory( size_t sizeX_, size_t sizeY_ )
{
	sizeX = sizeX_;
	sizeY = sizeY_;

	slotUsed = new bool*[sizeX];
	for ( size_t curX=0; curX<sizeX; ++curX ) {
		slotUsed[ curX ] = new bool[sizeY];
		for ( size_t curY=0; curY<sizeY; ++curY ) {
			slotUsed[ curX ][ curY ] = false;
		}
	}
}

Inventory::~Inventory()
{
	for ( size_t curX=0; curX<sizeX; ++curX ) {
		delete[] slotUsed[ curX ];
	}
	delete[] slotUsed;
}

bool Inventory::insertItem( Item *item )
{
	size_t itemSizeX = item->getSizeX();
	size_t itemSizeY = item->getSizeY();
	
	bool foundPosition = false;
	size_t foundX;
	size_t foundY;

	// look for next free position
	for ( size_t freeX=0; freeX<sizeX-itemSizeX+1 && !foundPosition; ++freeX ) {
		for ( size_t freeY=0; freeY<sizeY-itemSizeY+1 && !foundPosition; ++freeY ) {
			if ( hasSufficientSpaceAt( freeX, freeY, itemSizeX, itemSizeY ) ) {
				foundPosition = true;
				foundX = freeX;
				foundY = freeY;
			}
		}
	}
	
	if ( ! foundPosition ) {
		return false;
	}
	
	InventoryItem *newInvItem = new InventoryItem( item, foundX, foundY );
	insertItemAt( newInvItem, foundX, foundY );
}

bool Inventory::hasSufficientSpaceAt( size_t inventoryPosX, size_t inventoryPosY, size_t itemSizeX, size_t itemSizeY ) const
{
	size_t inventoryMaxX = inventoryPosX + itemSizeX - 1;
	size_t inventoryMaxY = inventoryPosY + itemSizeY - 1;
	if ( (inventoryMaxX >= sizeX) || (inventoryMaxY >= sizeY) ) {
		return false;
	}
 
	for ( size_t curX=inventoryPosX; curX<=inventoryMaxX; ++curX ) {
		for ( size_t curY=inventoryPosY; curY<=inventoryMaxY; ++curY ) {
			if ( slotUsed[ curX ][ curY ] ) {
				return false;
			}
		}
	}
	
	return true;
}

std::vector<InventoryItem*> Inventory::getBackpackItems() const
{
	return backpackItems;
}

bool Inventory::isPositionFree( size_t invPosX, size_t invPosY ) const
{
	if ( invPosX >= sizeX || invPosY >= sizeY ) {
		return false;
	}

	return ( !slotUsed[invPosX][invPosY] );
}

InventoryItem* Inventory::getItemAt( size_t invPosX, size_t invPosY )
{
	assert( ! isPositionFree( invPosX, invPosY ) );

	size_t numBackItems = backpackItems.size();
	for ( size_t curBackItemNr=0; curBackItemNr<numBackItems; ++curBackItemNr ) {
		InventoryItem *curItem = backpackItems[ curBackItemNr ];
		size_t itemPosX = curItem->getInventoryPosX();
		size_t itemPosY = curItem->getInventoryPosY();
		size_t itemSizeX = curItem->getItem()->getSizeX();
		size_t itemSizeY = curItem->getItem()->getSizeY();
		
		if ( itemPosX <= invPosX && itemPosX + itemSizeX > invPosX
		     && itemPosY <= invPosY && itemPosY + itemSizeY > invPosY ) {
			return curItem;
		}
	}
	
	// should have found an item so should never reach here
	abort();
}

bool Inventory::containsItem( InventoryItem *inventoryItem ) const
{
	for ( size_t curBackpackItemNr=0; curBackpackItemNr<backpackItems.size(); ++curBackpackItemNr ) {
		if ( backpackItems[ curBackpackItemNr ] == inventoryItem ) {
			return true;
		}
	}
	return false;
}

void Inventory::removeItem( InventoryItem *inventoryItem )
{
	for ( size_t curBackpackItemNr=0; curBackpackItemNr<backpackItems.size(); ++curBackpackItemNr ) {
		if ( backpackItems[ curBackpackItemNr ] == inventoryItem ) {
			for ( size_t curX=inventoryItem->getInventoryPosX(); curX<inventoryItem->getInventoryPosX() + inventoryItem->getSizeX(); ++curX ) {
				for ( size_t curY=inventoryItem->getInventoryPosY(); curY<inventoryItem->getInventoryPosY() + inventoryItem->getSizeY(); ++curY ) {
					slotUsed[curX][curY] = false;
				}
			}
			backpackItems[ curBackpackItemNr ] = backpackItems[ backpackItems.size() - 1 ];
			backpackItems.resize( backpackItems.size() - 1 );
		}
	}
}

void Inventory::insertItemAt( InventoryItem *inventoryItem, size_t invPosX, size_t invPosY )
{
	assert( hasSufficientSpaceAt( invPosX, invPosY, inventoryItem->getSizeX(), inventoryItem->getSizeY() ) );
	inventoryItem->setInventoryPos( invPosX, invPosY );
	backpackItems.push_back( inventoryItem );
	
	size_t inventoryMaxX = invPosX + inventoryItem->getSizeX() - 1;
	size_t inventoryMaxY = invPosY + inventoryItem->getSizeY() - 1;
	
	for ( size_t curX=invPosX; curX<=inventoryMaxX; ++curX ) {
		for ( size_t curY=invPosY; curY<=inventoryMaxY; ++curY ) {
			slotUsed[ curX ][ curY ] = true;
		}
	}
}