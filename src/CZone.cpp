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

#include "CZone.h"

#include "globals.h"

#include "CLuaFunctions.h"
#include "CNPC.h"
#include "interactionpoint.h"
#include "interactionregion.h"
#include "shop.h"
#include "callindirection.h"
#include "textwindow.h"
#include "CLuaInterface.h"

#include <cassert>
#include <memory>

extern Player character;

CZone::CZone()
	: groundLoot( &character ),
	  mapLoaded( false )
{
}

void CZone::DrawZone()
{
	DrawTiles(); // draw the tiles (ground) first.
	DrawEnvironment(); // then the environment.. cliffs, trees, stones, water ... you name it.
	DrawShadows(); // then draw the shadows (not shadows from environment objects but, cloudy areas, darker places etc).
}

void CZone::LoadZone(std::string file)
{
	zoneName = file;
	Globals::allZones[ file ] = this;
	LuaFunctions::executeLuaScript( std::string("DawnInterface.setCurrentZone( \"").append( zoneName ).append("\");") );
	ZoneTiles.LoadTextureMap( std::string( file ).append ( ".textures" ) );
	ZoneEnvironment.LoadTextureMap( std::string( file ).append ( ".textureenvironment" ) ,true);
	ZoneShadow.LoadTextureMap( std::string( file ).append ( ".textureshadow" ) );

	LoadMap( std::string( file ).append ( ".tilemap" ) );
	LoadEnvironment( std::string( file ).append ( ".environmentmap" ) );
	LoadShadow( std::string( file ).append ( ".shadowmap" ) );
	LoadCollisions( std::string( file ).append ( ".collisionmap" ) );
	LuaFunctions::executeLuaFile( std::string( file ).append( ".spawnpoints" ) );
	
	mapLoaded = true;
}

int CZone::LoadCollisions(std::string file)
{
	char buf[255];
	int CR_x = 0, CR_y = 0, CR_h = 0, CR_w = 0;

	std::ifstream input_file(file.c_str());

	// open the texturemap-file, if not give us an error in stdout.txt.
	if (!input_file) {
		std::cout << "ERROR opening file " << file << std::endl << std::endl;
		return -1;
	}

	while (input_file.getline (buf, 255)) {
		if (buf[0] != '#' && buf[0] != '\r' && buf[0] != '\0' &&
		        buf[0] != '\n' && strlen(buf) != 0) {
			sscanf(buf, "%d %d %d %d", &CR_x, &CR_y, &CR_h, &CR_w);
			CollisionMap.push_back(sCollisionMap(CR_x,CR_y,CR_h,CR_w));
		}
	}
	input_file.close();

	return 0;
}

int CZone::LoadEnvironment(std::string file)
{
	std::ifstream input_file(file.c_str());
	char buf[255];
	int texture_id = 0, x_pos = 0, y_pos = 0, z_pos = 0;
	float transparency, red, green, blue, x_scale, y_scale;
	int count = 0;

	// open the texturemap-file, if not give us an error in stdout.txt.
	if (!input_file) {
		std::cout << "ERROR opening file " << file << std::endl << std::endl;
		return -1;
	}

	for (count = 0 ; input_file.getline (buf, 255) ; ++count) {
		if (buf[0] != '#' && buf[0] != '\r' && buf[0] != '\0' &&
		        buf[0] != '\n' && strlen(buf) != 0) {
			sscanf(buf,"%d %d %d %f %f %f %f %f %f %d", &x_pos, &y_pos, &texture_id, &transparency, &red, &green, &blue, &x_scale, &y_scale, &z_pos);
			EnvironmentMap.push_back(sEnvironmentMap(x_pos,y_pos,texture_id,transparency, red, green, blue, x_scale, y_scale, z_pos));
		}
	}

	input_file.close();

    std::sort(EnvironmentMap.begin(), EnvironmentMap.end());

	return 0;
}

int CZone::LoadShadow(std::string file)
{
	std::ifstream input_file(file.c_str());
	char buf[255];
	int texture_id = 0, x_pos = 0, y_pos = 0;
	float transparency, red, green, blue, x_scale, y_scale;
	int count = 0;

	// open the shadowmap-file, if not give us an error in stdout.txt.
	if (!input_file) {
		std::cout << "ERROR opening file " << file << std::endl << std::endl;
		return -1;
	}

	for (count = 0 ; input_file.getline (buf, 255) ; ++count) {
		if (buf[0] != '#' && buf[0] != '\r' && buf[0] != '\0' &&
		        buf[0] != '\n' && strlen(buf) != 0) {
			sscanf(buf, "%d %d %d %f %f %f %f %f %f", &x_pos, &y_pos, &texture_id, &transparency, &red, &green, &blue, &x_scale, &y_scale);
			// the old shadowmap here, keeping it a while. ShadowMap.push_back(sShadowMap(x_pos,y_pos,texture_id, transparency, red, green, blue));
			ShadowMap.push_back(sEnvironmentMap(x_pos,y_pos,texture_id,transparency, red, green, blue, x_scale, y_scale, 0));
			count++;
		}
	}

	input_file.close();

	return 0;
}

int CZone::LoadMap(std::string file)
{
	std::ifstream input_file(file.c_str());
	char buf[255];
	int texture_id = 0, x_pos = 0, y_pos = 0;
	int count = 0;

	// open the texturemap-file, if not give us an error in stdout.txt.
	if (!input_file) {
		std::cout << "ERROR opening file " << file << std::endl << std::endl;
		return -1;
	}

	for (count = 0 ; input_file.getline (buf, 255) ; ++count) {
		if (buf[0] != '#' && buf[0] != '\r' && buf[0] != '\0' &&
		        buf[0] != '\n' && strlen(buf) != 0) {
			sscanf(buf, "%d %d %d", &x_pos, &y_pos, &texture_id);
			TileMap.push_back(sTileMap(x_pos,y_pos,texture_id));
			count++;
		}
	}
	std::sort(TileMap.begin(), TileMap.end()); // sort our vector based on texture, less calls to glBindTexture() = more performance.

	input_file.close();
	
	return 0;
}

bool CZone::zoneDataLoaded() const
{
	return mapLoaded;
}

void CZone::DrawTiles()
{
	for (unsigned int x=0 ; x < TileMap.size() ; x++) {
		ZoneTiles.DrawTexture(TileMap[x].x_pos,TileMap[x].y_pos,TileMap[x].id);
	}
}

void CZone::DrawEnvironment()
{
	for (unsigned int x=0 ; x < EnvironmentMap.size() ; x++) {
		ZoneEnvironment.DrawTexture(EnvironmentMap[x].x_pos,
		                            EnvironmentMap[x].y_pos,EnvironmentMap[x].id,
		                            EnvironmentMap[x].transparency, EnvironmentMap[x].red,
		                            EnvironmentMap[x].green, EnvironmentMap[x].blue,
		                            EnvironmentMap[x].x_scale, EnvironmentMap[x].y_scale);
	}
}

void CZone::DrawShadows()
{
	for (unsigned int x=0 ; x < ShadowMap.size() ; x++) {
		ZoneShadow.DrawTexture(ShadowMap[x].x_pos,ShadowMap[x].y_pos,ShadowMap[x].id,
		                       ShadowMap[x].transparency, ShadowMap[x].red,
		                       ShadowMap[x].green, ShadowMap[x].blue, ShadowMap[x].x_scale,
		                       ShadowMap[x].y_scale);
	}
}

int CZone::LocateTile(int x, int y)
{
	for (unsigned int t=0;t<TileMap.size();t++) {
		if ((TileMap[t].x_pos+ZoneTiles.texture[TileMap[t].id].width > x) &&
		        (TileMap[t].x_pos < x)) {
			if ((TileMap[t].y_pos+ZoneTiles.texture[TileMap[t].id].height > y) &&
			        (TileMap[t].y_pos < y)) {
				return t;
			}
		}
	}
	return -1;
}

void CZone::ChangeTile(int iId, int texture)
{
	if (iId >= 0) {
		TileMap[iId].id = texture;
	}
}

void CZone::DeleteTile(int iId)
{
	if (iId >= 0) {
		TileMap[iId].id = 0;
	}
}

void CZone::AddEnvironment(int x_pos, int y_pos, int texture)
{
	// add environment to our environmentvector.
	// x and y cords and devide and substract the height and width of the image so we place the texture
	// in the middle of the cursor.
	// IF the environmenttexture has an collision_box we also push that info into the collisionvector.
	EnvironmentMap.push_back(sEnvironmentMap(x_pos-(ZoneEnvironment.texture[texture].width/2),
	                         y_pos-(ZoneEnvironment.texture[texture].height/2),texture,
	                         1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0));
	if (ZoneEnvironment.texture[texture].contains_collision_box == true) {
		CollisionMap.push_back(sCollisionMap(x_pos-(
		                                         ZoneEnvironment.texture[texture].width/2)+
		                                     ZoneEnvironment.texture[texture].collision_box.x,
		                                     y_pos-(ZoneEnvironment.texture[texture].height/2)+
		                                     ZoneEnvironment.texture[texture].collision_box.y,
		                                     ZoneEnvironment.texture[texture].collision_box.h,
		                                     ZoneEnvironment.texture[texture].collision_box.w));
	}
}

void CZone::AddShadow(int x_pos, int y_pos, int texture)
{
	// the old shadowmap here, keeping it a while... ShadowMap.push_back(sShadowMap(x_pos-(ZoneShadow.texture[texture].width/2),y_pos-(ZoneShadow.texture[texture].height/2),texture, 1.0f, 1.0f, 1.0f, 1.0f));
	ShadowMap.push_back(sEnvironmentMap(x_pos-(ZoneShadow.texture[texture].width/2),
	                                    y_pos-(ZoneShadow.texture[texture].height/2),texture, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0));
}

void CZone::AddCollisionbox(int x_pos, int y_pos)
{
	CollisionMap.push_back(sCollisionMap(x_pos,y_pos,100,100));
}

int CZone::DeleteEnvironment(int x, int y)
{
	for (unsigned int t=0;t<EnvironmentMap.size();t++) {
		if ((EnvironmentMap[t].x_pos+ZoneEnvironment.texture[EnvironmentMap[t].id].width > x) &&
		        (EnvironmentMap[t].x_pos < x)) {
			if ((EnvironmentMap[t].y_pos+ZoneEnvironment.texture[EnvironmentMap[t].id].height > y) &&
			        (EnvironmentMap[t].y_pos < y)) {
				EnvironmentMap.erase(EnvironmentMap.begin()+t);
				return 0;
			}
		}
	}
	return 1;
}

int CZone::LocateEnvironment(int x, int y)
{
	for (unsigned int t=0;t<EnvironmentMap.size();t++) {
		if ((EnvironmentMap[t].x_pos+ZoneEnvironment.texture[EnvironmentMap[t].id].width > x) &&
		        (EnvironmentMap[t].x_pos < x)) {
			if ((EnvironmentMap[t].y_pos+ZoneEnvironment.texture[EnvironmentMap[t].id].height > y) &&
			        (EnvironmentMap[t].y_pos < y)) {
				return t;
			}
		}
	}
	return -1;
}

int CZone::LocateShadow(int x, int y)
{
	for (unsigned int t=0;t<ShadowMap.size();t++) {
		if ((ShadowMap[t].x_pos+ZoneShadow.texture[ShadowMap[t].id].width > x) &&
		        (ShadowMap[t].x_pos < x)) {
			if ((ShadowMap[t].y_pos+ZoneShadow.texture[ShadowMap[t].id].height > y) &&
			        (ShadowMap[t].y_pos < y)) {
				return t;
			}
		}
	}
	return -1;
}

int CZone::DeleteShadow(int x, int y)
{
	for (unsigned int t=0;t<ShadowMap.size();t++) {
		if ((ShadowMap[t].x_pos+ZoneShadow.texture[ShadowMap[t].id].width > x) &&
		        (ShadowMap[t].x_pos < x)) {
			if ((ShadowMap[t].y_pos+ZoneShadow.texture[ShadowMap[t].id].height > y) &&
			        (ShadowMap[t].y_pos < y)) {
				ShadowMap.erase(ShadowMap.begin()+t);
				return 0;
			}
		}
	}
	return 1;
}

int CZone::LocateCollisionbox(int x, int y)
{
	for (unsigned int t=0;t<CollisionMap.size();t++) {
		if ((CollisionMap[t].CR.x+CollisionMap[t].CR.w > x) && (CollisionMap[t].CR.x < x)) {
			if ((CollisionMap[t].CR.y+CollisionMap[t].CR.h > y) && (CollisionMap[t].CR.y < y)) {
				return t;
			}
		}
	}
	return -1;
}


int CZone::DeleteCollisionbox(int x, int y)
{
	for (unsigned int t=0;t<CollisionMap.size();t++) {
		if ((CollisionMap[t].CR.x+CollisionMap[t].CR.w > x) && (CollisionMap[t].CR.x < x)) {
			if ((CollisionMap[t].CR.y+CollisionMap[t].CR.h > y) && (CollisionMap[t].CR.y < y)) {
				CollisionMap.erase(CollisionMap.begin()+t);
				return 0;
			}
		}
	}
	return -1;
}

std::vector<CNPC*> CZone::getNPCs()
{
	return npcs;
}

void CZone::addNPC( CNPC *npcToAdd )
{
	npcs.push_back( npcToAdd );
}

void CZone::removeNPC( CNPC *npcToDelete )
{
		for ( size_t curNPCNr=0; curNPCNr<npcs.size(); ++curNPCNr ) {
			CNPC *curNPC = npcs[ curNPCNr ];
			if ( curNPC == npcToDelete ) {
				curNPC->markAsDeleted();
				break;
			}
		}
}

void CZone::cleanupNPCList()
{
	size_t curNPCNr = 0;
	while ( curNPCNr < npcs.size() ) {
		CNPC *curNPC = npcs[ curNPCNr ];
		if ( curNPC->isMarkedAsDeletable() ) {
			npcs.erase( npcs.begin() + curNPCNr );
			// TODO: delete curNPC. There seem to be some problems at the moment.
			//delete curNPC;
		} else {
			++curNPCNr;
		}
	}
}

std::vector<InteractionPoint*> CZone::getInteractionPoints()
{
	return interactionPoints;
}

void CZone::addInteractionPoint( InteractionPoint *interactionPointToAdd )
{
	interactionPoints.push_back( interactionPointToAdd );
}

void CZone::cleanupInteractionList()
{
	size_t curInteractionNr = 0;
	while ( curInteractionNr < interactionPoints.size() ) {
		InteractionPoint *curInteraction = interactionPoints[ curInteractionNr ];
		if ( curInteraction->isMarkedDeletable() ) {
			// return from list
			interactionPoints[ curInteractionNr ] = interactionPoints[ interactionPoints.size() - 1 ];
			interactionPoints.resize( interactionPoints.size() - 1 );
			delete curInteraction;
		} else {
			++curInteractionNr;
		}
	}
}

void CZone::purgeInteractionList()
{
	for ( size_t curInteractionNr=0; curInteractionNr < interactionPoints.size(); ++curInteractionNr ) {
		InteractionPoint *curInteraction = interactionPoints[ curInteractionNr ];
		delete curInteraction;
	}
	interactionPoints.resize(0);
}

std::vector<InteractionRegion*> CZone::getInteractionRegions()
{
	return interactionRegions;
}

void CZone::addInteractionRegion( InteractionRegion *interactionRegionToAdd )
{
	interactionRegions.push_back( interactionRegionToAdd );
}

void CZone::cleanupInteractionRegionList()
{
	size_t curInteractionNr = 0;
	while ( curInteractionNr < interactionRegions.size() ) {
		InteractionRegion *curInteraction = interactionRegions[ curInteractionNr ];
		if ( curInteraction->isMarkedDeletable() ) {
			// return from list
			interactionRegions[ curInteractionNr ] = interactionRegions[ interactionRegions.size() - 1 ];
			interactionRegions.resize( interactionRegions.size() - 1 );
			delete curInteraction;
		} else {
			++curInteractionNr;
		}
	}
}

void CZone::purgeInteractionRegionList()
{
	for ( size_t curInteractionNr=0; curInteractionNr < interactionRegions.size(); ++curInteractionNr ) {
		InteractionRegion *curInteraction = interactionRegions[ curInteractionNr ];
		delete curInteraction;
	}
	interactionPoints.resize(0);
}

GroundLoot* CZone::getGroundLoot()
{
	return &groundLoot;
}

extern std::auto_ptr<Shop> shopWindow;

std::string CZone::getLuaSaveText() const
{
	std::ostringstream oss;
	oss << "DawnInterface.setCurrentZone( \"" << zoneName << "\" );" << std::endl;

	// save call indirections (must be added before spawnpoints since used there)
	oss << "-- event handlers" << std::endl;
	for ( size_t curEventHandlerNr=0; curEventHandlerNr<eventHandlers.size(); ++curEventHandlerNr ) {
		CallIndirection *curEventHandler = eventHandlers[ curEventHandlerNr ];
		std::string eventHandlerSaveText = curEventHandler->getLuaSaveText();
		oss << eventHandlerSaveText;
	}
	
	// save all spawnpoints
	oss << "-- spawnpoints" << std::endl;
	for ( size_t curNpcNr=0; curNpcNr < npcs.size(); ++curNpcNr ) {
		CNPC *curNPC = npcs[ curNpcNr ];
		// save cur npc
		std::string npcSaveText = curNPC->getLuaSaveText();
		oss << npcSaveText;
	}
	
	// save shop data (this ought to be put into the shop somehow as soon as shops are customizable)
	oss << "-- shops" << std::endl;
	oss << "local curShop = DawnInterface.addShop();" << std::endl;
	for ( size_t curTab=0; curTab<3; ++curTab ) {
		for ( size_t curItemNr=0; curItemNr < shopWindow->shopkeeperInventory[curTab].size(); ++curItemNr ) {
			oss << "curShop:addItem( itemDatabase[\"" << shopWindow->shopkeeperInventory[curTab][curItemNr]->getItem()->getID() << "\"] );" << std::endl;
		}
	}

	// save interaction points
	oss << "-- interaction points" << std::endl;
	for ( size_t curInteractionNr=0; curInteractionNr < interactionPoints.size(); ++curInteractionNr ) {
		InteractionPoint *curInteractionPoint = interactionPoints[ curInteractionNr ];
		std::string interactionSaveText = curInteractionPoint->getLuaSaveText();
		oss << interactionSaveText;
	}

	// save interaction regions
	oss << "-- interaction regions" << std::endl;
	for ( size_t curInteractionNr=0; curInteractionNr < interactionRegions.size(); ++curInteractionNr ) {
		InteractionRegion *curInteractionRegion = interactionRegions[ curInteractionNr ];
		std::string interactionSaveText = curInteractionRegion->getLuaSaveText();
		oss << interactionSaveText;
	}
	
	// save ground loot
	oss << "-- ground loot" << std::endl;
	for ( size_t curGroundItemNr=0; curGroundItemNr < groundLoot.groundItems.size(); ++curGroundItemNr ) {
		sGroundItems curGroundItem = groundLoot.groundItems[ curGroundItemNr ];
		Item *item = curGroundItem.item;
		if ( dynamic_cast<GoldHeap*>( item ) != NULL ) {
			GoldHeap *goldHeap = dynamic_cast<GoldHeap*>( item );
			oss << "DawnInterface.restoreGroundGold( "
			             << goldHeap->numCoins() << ", "
			             << curGroundItem.xpos << ", "
			             << curGroundItem.ypos << " );" << std::endl;
		} else {
			oss << "DawnInterface.restoreGroundLootItem( "
			             << "itemDatabase[ \"" << item->getID() << "\" ], "
			             << curGroundItem.xpos << ", "
			             << curGroundItem.ypos << " );" << std::endl;
		}
	}
	
	return oss.str();
}

void CZone::addEventHandler( CallIndirection *newEventHandler )
{
	eventHandlers.push_back( newEventHandler );
}

std::string CZone::getZoneName() const
{
	return zoneName;
}

void CZone::findCharacter( CCharacter *character, bool &found, size_t &foundPos ) const
{
	for ( size_t curNpcNr=0; curNpcNr < npcs.size(); ++curNpcNr ) {
		if ( npcs[ curNpcNr ] == character ) {
			found = true;
			foundPos = curNpcNr;
			return;
		}
	}
	found = false;
}

void CZone::findInteractionPoint( InteractionPoint *interactionPoint, bool &found, size_t &foundPos ) const
{
	for ( size_t curInteractionNr=0; curInteractionNr < interactionPoints.size(); ++curInteractionNr ) {
		if ( interactionPoints[ curInteractionNr ] == interactionPoint ) {
			found = true;
			foundPos = curInteractionNr;
			return;
		}
	}
	found = false;
}

void CZone::findInteractionRegion( InteractionRegion *interactionRegion, bool &found, size_t &foundPos ) const
{
	for ( size_t curInteractionNr=0; curInteractionNr < interactionRegions.size(); ++curInteractionNr ) {
		if ( interactionRegions[ curInteractionNr ] == interactionRegion ) {
			found = true;
			foundPos = curInteractionNr;
			return;
		}
	}
	found = false;
}

void CZone::findEventHandler( CallIndirection *eventHandler, bool &found, size_t &foundPos ) const
{
	for ( size_t curEventHandlerNr=0; curEventHandlerNr < eventHandlers.size(); ++curEventHandlerNr ) {
		if ( eventHandlers[ curEventHandlerNr ] == eventHandler ) {
			found = true;
			foundPos = curEventHandlerNr;
			return;
		}
	}
	found = false;
}

CCharacter* CZone::getCharacterPointer( size_t posInArray ) const
{
	// use checked access since we access from lua and lots of stuff could be wrong
	return npcs.at( posInArray );
}

InteractionPoint* CZone::getInteractionPointPointer( size_t posInArray ) const
{
	// use checked access since we access from lua and lots of stuff could be wrong
	return interactionPoints.at( posInArray );
}

InteractionRegion* CZone::getInteractionRegionPointer( size_t posInArray ) const
{
	// use checked access since we access from lua and lots of stuff could be wrong
	return interactionRegions.at( posInArray );
}

CallIndirection* CZone::getEventHandlerPointer( size_t posInArray ) const
{
	// use checked access since we access from lua and lots of stuff could be wrong
	return eventHandlers.at( posInArray );
}

namespace DawnInterface
{
	std::string getAllZonesSaveText()
	{
		std::ostringstream oss;
		for ( std::map< std::string, CZone* >::iterator it = Globals::allZones.begin(); it != Globals::allZones.end(); ++it ) {
			oss << it->second->getLuaSaveText();
		}

		return oss.str();
	}
	
	std::string getReenterCurrentZoneText()
	{
		std::ostringstream oss;
		oss << "DawnInterface.enterZone( \"" << Globals::getCurrentZone()->getZoneName() << "\", " << getPlayer()->getXPos() << ", " << getPlayer()->getYPos() << " );" << std::endl;

		return oss.str();
	}
	
	void enterZone( std::string zoneName, int enterX, int enterY )
	{
		CZone *newZone = Globals::allZones[ zoneName ];
		if ( newZone == NULL ) {
			newZone = new CZone();
			Globals::allZones[ zoneName ] = newZone;
		}
		if ( ! newZone->zoneDataLoaded() ) {
			newZone->LoadZone( zoneName );
		}
		Globals::setCurrentZone( newZone );
		getPlayer()->setPosition( enterX, enterY );
	}
	
	void restoreGroundLootItem( Item *item, int xPos, int yPos )
	{
		Globals::getCurrentZone()->getGroundLoot()->addItem( xPos, yPos, item );
	}
	
	void restoreGroundGold( int amount, int xPos, int yPos )
	{
		Globals::getCurrentZone()->getGroundLoot()->addItem( xPos, yPos, new GoldHeap( amount ) );
	}

	std::string getItemReferenceRestore( CCharacter *character )
	{
		if ( character == NULL ) {
			return "nil;";
		}
		for ( std::map< std::string, CZone* >::iterator it = Globals::allZones.begin(); it != Globals::allZones.end(); ++it ) {
			CZone *curZone = it->second;
			bool found;
			size_t foundPos;
			curZone->findCharacter( character, found, foundPos );
			if ( found ) {
				std::ostringstream oss;
				oss << "DawnInterface.restoreCharacterReference( \"" << curZone->getZoneName() << "\", " << foundPos << " )";
				return oss.str();
			}
		}
		// not found
		abort();
		dawn_debug_fatal( "could not find character in any of the zones" );
		abort();
	}
	
	std::string getItemReferenceRestore( InteractionPoint *interactionPoint )
	{
		if ( interactionPoint == NULL ) {
			return "nil;";
		}
		for ( std::map< std::string, CZone* >::iterator it = Globals::allZones.begin(); it != Globals::allZones.end(); ++it ) {
			CZone *curZone = it->second;
			bool found;
			size_t foundPos;
			curZone->findInteractionPoint( interactionPoint, found, foundPos );
			if ( found ) {
				std::ostringstream oss;
				oss << "DawnInterface.restoreInteractionPointReference( \"" << curZone->getZoneName() << "\", " << foundPos << " )";
				return oss.str();
			}
		}
		// not found
		dawn_debug_fatal( "could not find interaction point in any of the zones" );
		abort();
	}
	
	std::string getItemReferenceRestore( CallIndirection *eventHandler )
	{
		if ( eventHandler == NULL ) {
			return "nil;";
		}
		for ( std::map< std::string, CZone* >::iterator it = Globals::allZones.begin(); it != Globals::allZones.end(); ++it ) {
			CZone *curZone = it->second;
			bool found;
			size_t foundPos;
			curZone->findEventHandler( eventHandler, found, foundPos );
			if ( found ) {
				std::ostringstream oss;
				oss << "DawnInterface.restoreEventHandlerReference( \"" << curZone->getZoneName() << "\", " << foundPos << " )";
				return oss.str();
			}
		}
		// not found
		dawn_debug_fatal( "could not find event handler in any of the zones" );
		abort();
	}
	
	std::string getItemReferenceRestore( Shop *shop )
	{
		return "DawnInterface.addShop(); -- shops not really searchable, yet";
	}
	
	std::string getItemReferenceRestore( TextWindow *textWindow )
	{
		return "DawnInterface.createTextWindow(); -- text windows are not restored";
	}

	CCharacter* restoreCharacterReference( std::string zoneName, int posInArray )
	{
		CZone *correctZone = Globals::allZones[ zoneName ];
		assert( correctZone != NULL );
		return correctZone->getCharacterPointer( posInArray );
	}

	InteractionPoint* restoreInteractionPointReference( std::string zoneName, int posInArray )
	{
		CZone *correctZone = Globals::allZones[ zoneName ];
		assert( correctZone != NULL );
		return correctZone->getInteractionPointPointer( posInArray );
	}

	CallIndirection* restoreEventHandlerReference( std::string zoneName, int posInArray )
	{
		CZone *correctZone = Globals::allZones[ zoneName ];
		assert( correctZone != NULL );
		return correctZone->getEventHandlerPointer( posInArray );
	}
}

