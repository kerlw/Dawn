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

#include "main.h"

#include "threadObject/Thread.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <csignal>

#ifdef _WIN32
#include <windows.h> // Nothing uses this, perhaps it could be removed?
#endif

#include "GLee/GLee.h" // OpenGL Easy Extention Library

#include <SDL/SDL.h> // SDL
#include <SDL/SDL_opengl.h> // SDL OpenGL
#include <SDL/SDL_image.h> // SDL Image library (image formats: BMP, GIF, JPEG, LBM, PCX, PNG, PNM, TGA, TIFF, XCF, XPM, XV)
#include <SDL/SDL_getenv.h>

#include <GL/gl.h> // OpenGL
#include <GL/glu.h> // OpenGL Utility Library. This will have to be changed in updated versions, as it changes for different OS's

#include "GLFT_Font.h"
#include "pnglite/pnglite.h"

#include "CTexture.h"
#include "CZone.h"
#include "CInterface.h"
#include "CCharacter.h"
#include "CEditor.h"
#include "CMessage.h"
#include "InventoryScreen.h"
#include "cameraFocusHandler.h"
#include "utils.h"
#include "tooltip.h"
#include "ActionBar.h"
#include "Spellbook.h"
#include "BuffWindow.h"
#include "shop.h"
#include "GroundLoot.h"
#include "CLuaFunctions.h"
#include "CSpell.h"
#include "debug.h"
#include "CharacterInfoScreen.h"
#include "item.h"
#include "interactionpoint.h"
#include "interactionregion.h"
#include "textwindow.h"
#include "questwindow.h"
#include "optionswindow.h"
#include "loadingscreen.h"
#include "globals.h"
#include "FramesBase.h"

#ifdef _WIN32
#define SDLK_PRINT 316 // this is because Windows printscreen doesn't match the SDL predefined keycode.
#endif

/* Global settings now reside in the
   dawn_configuration namespace, variables
   are added to this across multiple files.
   current headers adding to it:
   debug.h
 */
namespace dawn_configuration {
	bool fullscreenenabled = true;
	int screenWidth = 1024;
	int screenHeight = 768;
	int bpp = 32;
}
// FIXME: This is a temp hack until the
// 	objects dont need those variables.
//	david: I'll have this sorted pretty
//	quick.
int RES_X = dawn_configuration::screenWidth;
int RES_Y = dawn_configuration::screenHeight;

int world_x = 0, world_y = 0;
int mouseX, mouseY;
int done = 0;
CMessage message;

SDL_Surface *screen;
Player character;
cameraFocusHandler focus(dawn_configuration::screenWidth, dawn_configuration::screenHeight);

CEditor Editor;

CInterface GUI;

extern std::vector<TextWindow*> allTextWindows;

std::vector<FramesBase*> activeFrames;

bool KP_interrupt = false;
bool KP_select_next = false;
bool KP_screenshot = false;
bool KP_toggle_showCharacterInfo = false;
bool KP_toggle_showInventory = false;
bool KP_toggle_showSpellbook = false;
bool KP_toggle_showQuestWindow = false;
bool KP_toggle_showOptionsWindow = false;

float lastframe,thisframe;           // FPS Stuff
int ff, fps;                         // FPS Stuff

GLFT_Font *fpsFont;
std::auto_ptr<CharacterInfoScreen> characterInfoScreen;
std::auto_ptr<InventoryScreen> inventoryScreen;
std::auto_ptr<ActionBar> actionBar;
std::auto_ptr<Spellbook> spellbook;
std::auto_ptr<BuffWindow> buffWindow;
std::auto_ptr<QuestWindow> questWindow;
std::auto_ptr<OptionsWindow> optionsWindow;
std::auto_ptr<Shop> shopWindow;

std::vector<CSpellActionBase*> activeSpellActions;

namespace Globals
{
	std::map< std::string, CZone* > allZones;
}

void enqueueActiveSpellAction( CSpellActionBase *spellaction )
{
	activeSpellActions.push_back( spellaction );
}

void cleanupActiveSpellActions()
{
	size_t curActiveNr = 0;
	while ( curActiveNr < activeSpellActions.size() ) {
		if ( activeSpellActions[ curActiveNr ]->isEffectComplete() ) {
			delete activeSpellActions[ curActiveNr ];
			activeSpellActions.erase( activeSpellActions.begin() + curActiveNr );
		} else {
			++curActiveNr;
		}
	}
}

static bool HandleCommandLineAurguments(int argc, char** argv)
{
	bool run_game = true;
	std::string executable(argv[0]);
#ifdef _WIN32
	freopen( "CON", "wt", stdout ); // Redirect stdout to the command line
#endif
	for (int i=1 ; i < argc ; ++i) {
		std::string currentarg(argv[i]);
		if (currentarg == "-f" || currentarg == "--fullscreen") {
			dawn_configuration::fullscreenenabled = true;
			run_game = true;
		} else if (currentarg == "-w" || currentarg == "--window") {
			dawn_configuration::fullscreenenabled = false;
			run_game = true;
		} else if (currentarg == "-h" || currentarg == "--help") {
			std::cout << "Dawn-RPG Startup Parameters" <<
			          std::endl << std::endl <<
			          " -f, --fullscreen         Run Dawn in fullscreen mode" <<
			          std::endl <<
			          " -w, --window             Run Dawn inside a window" <<
			          std::endl <<
			          " -h, --help               Show this help screen" <<
			          std::endl;
			run_game = false;
		} else {
			std::cout << std::endl <<"\"" << currentarg <<
			          "\" is not a recognised option" << std::endl << std::endl <<
			          "Please type \"" << executable <<
			          " -h\" for all available options" << std::endl <<
			          std::endl;
			run_game = false;
			break;
		}
	}
#ifdef _WIN32
	freopen( "stdout.txt", "wt", stdout ); // Redirect stdout back to the file
#endif
	return run_game;
}

namespace DawnInterface
{
	CNPC *addMobSpawnPoint( std::string mobID, int x_pos, int y_pos, int respawn_rate, int do_respawn )
	{
		CNPC *newMob = new CNPC(0, 0, 0, 0, 0);
		newMob->lifebar = NULL;
		newMob->baseOnType( mobID );
		newMob->setSpawnInfo( x_pos, y_pos, respawn_rate, do_respawn );
		newMob->setActiveGUI( &GUI );
		Globals::getCurrentZone()->addNPC( newMob );
		return newMob;
	}

	void removeMobSpawnPoint( CNPC *mobSpawnPoint )
	{
		Globals::getCurrentZone()->removeNPC( mobSpawnPoint );
	}

	Player* getPlayer()
	{
		return &character;
	}

	void setCurrentZone( std::string zoneName )
	{
		if ( Globals::allZones[ zoneName ] == NULL ) {
			Globals::allZones[ zoneName ] = new CZone();
		}
		CZone *newCurZone = Globals::allZones[ zoneName ];
		Globals::setCurrentZone( newCurZone );
	}
}

void DrawScene()
{
	//glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glTranslated(-focus.getX(), -focus.getY(),0);

	/* FIXME, TEMPORARY HACK: this is a quick fix, world_* should be
	 * removed once Editor and GUI don't need them */
	world_x = focus.getX();
	world_y = focus.getY();

	glColor4f(1.0f,1.0f,1.0f,1.0f);			// Full Brightness, 50% Alpha ( NEW )

	CZone *curZone = Globals::getCurrentZone();

	curZone->DrawZone();

	// draw items on the ground
    curZone->getGroundLoot()->draw();

    // draw the interactions on screen
    std::vector<InteractionPoint*> zoneInteractionPoints = curZone->getInteractionPoints();
	for ( size_t curInteractionNr=0; curInteractionNr<zoneInteractionPoints.size(); ++curInteractionNr ) {
		InteractionPoint *curInteraction = zoneInteractionPoints[ curInteractionNr ];
		curInteraction->draw();
	}

	character.Draw();

	// draw tooltips if we're holding left ALT key.
	curZone->getGroundLoot()->drawTooltip();

	// draw NPC (and if it's in target, their lifebar and name)
	std::vector<CNPC*> zoneNPCs = curZone->getNPCs();
	for (unsigned int x=0; x<zoneNPCs.size(); x++)
	{
		CNPC *curNPC = zoneNPCs[x];
		curNPC->Draw();
		if ( character.getTarget() == curNPC )
		{
            GUI.drawTargetedNPCText();
		}
	}

	for ( size_t curInteractionNr=0; curInteractionNr<zoneInteractionPoints.size(); ++curInteractionNr ) {
		InteractionPoint *curInteraction = zoneInteractionPoints[ curInteractionNr ];
		if ( curInteraction->isMouseOver(mouseX, mouseY) ) {
			curInteraction->drawInteractionSymbol( mouseX, mouseY, character.getXPos(), character.getYPos() );
		}
	}

	for ( size_t curActiveSpellNr = 0; curActiveSpellNr < activeSpellActions.size(); ++curActiveSpellNr ) {
		if ( ! activeSpellActions[ curActiveSpellNr ]->isEffectComplete() ) {
			activeSpellActions[ curActiveSpellNr ]->drawEffect();
		}
	}

	// check our FPS and output it
	thisframe=SDL_GetTicks();     // Count the FPS
	ff++;
	if ((thisframe-lastframe) > 1000) {
		fps=ff;
		ff=0;
		lastframe=thisframe;
	}

	if (Editor.isEnabled() ) {
		Editor.DrawEditor();
	} else {
		actionBar->draw();
		GUI.DrawInterface();
	}

	// loop through our vector of active frames and draw them. If they are in the vector they are visible...
	for ( size_t curFrame = 0; curFrame < activeFrames.size(); curFrame++ )
	{
	    activeFrames[ curFrame ]->draw( mouseX, mouseY );
	}

	buffWindow->draw();

	if ( actionBar->isMouseOver( mouseX, mouseY ) && !spellbook->hasFloatingSpell() )
	{
	    actionBar->drawSpellTooltip( mouseX, mouseY );
	}

    shopWindow->drawItemTooltip( mouseX, mouseY );
    shopWindow->drawFloatingSelection( world_x + mouseX, world_y + mouseY );
    inventoryScreen->drawItemTooltip( mouseX, mouseY );
    inventoryScreen->drawFloatingSelection( world_x + mouseX, world_y + mouseY );
    spellbook->drawFloatingSpell( mouseX, mouseY );

	// note: we need to cast fpsFont.getHeight to int since otherwise the whole expression would be an unsigned int
	//       causing overflow and not drawing the font if it gets negative

	// I've removed this text for now, just for a cleaner look. Enable it if you need some info while coding. /Arnestig
	// fpsFont->drawText(focus.getX(), focus.getY()+RES_Y - static_cast<int>(fpsFont->getHeight()), "FPS: %d     world_x: %2.2f, world_y: %2.2f      Xpos: %d, Ypos: %d      MouseX: %d, MouseY: %d",fps,focus.getX(),focus.getY(), character.x_pos, character.y_pos, mouseX, mouseY);
	// Only FPS
	fpsFont->drawText(focus.getX()+RES_X-100, focus.getY()+RES_Y - static_cast<int>(fpsFont->getHeight()), "FPS: %d",fps);

	message.DrawAll();
	message.DeleteDecayed();

	SDL_GL_SwapBuffers();
}


void dawn_init_signal_handlers()
{
	#ifndef WIN32
	dawn_debug_info("Initializing signal handlers...");
	signal( SIGFPE,  generalSignalHandler );
	signal( SIGSEGV, generalSignalHandler );
	signal( SIGBUS,  generalSignalHandler );
	signal( SIGABRT, generalSignalHandler );
    #endif
}

class DawnInitObject;
bool processFilesDirectly = true;
DawnInitObject *curTextureProcessor = NULL;
void processTextureInOpenGLThread( CTexture *texture, std::string texturefile, int textureIndex );

class DawnInitObject : public CThread
{
private:
	bool finished;
	CTexture *curTexture;
	std::string curTextureFile;
	int curTextureIndex;
	GLFT_Font *curFont;
	std::string curFontFile;
	unsigned int curFontSize;
	std::string progressString;
	double progress;
	CMutexClass accessMutex;
public:
	bool started;
	DawnInitObject()
	    : finished( false ),
	      curTexture( NULL ),
	      curFont( NULL ),
	      progressString( "" ),
	      progress( 0.0 ),
	      accessMutex(),
	      started( false )
	{
		SetThreadType(ThreadTypeEventDriven);
	}

	bool isFinished()
	{
		bool result = false;
		accessMutex.Lock();
		result = finished;
		accessMutex.Unlock();
		return result;
	}

	std::string getCurrentText()
	{
		return progressString;
	}

	double getProgress()
	{
		return progress;
	}

	void setCurrentTextureToProcess( CTexture *texture, std::string textureFile, int textureIndex )
	{
		accessMutex.Lock();
		curTexture = texture;
		curTextureFile = textureFile;
		curTextureIndex = textureIndex;
		accessMutex.Unlock();
		while ( curTexture != NULL ) {
			Sleep(1);
		}
	}

	void processCurTexture()
	{
		accessMutex.Lock();
		if ( curTexture != NULL ) {
			dawn_debug_info( "loading texture %s\n", curTextureFile.c_str());
			processFilesDirectly = true;
			curTexture->LoadIMG( curTextureFile, curTextureIndex );
			processFilesDirectly = false;
			curTexture = NULL;
		}
		accessMutex.Unlock();
	}

	void setCurrentFontToProcess( GLFT_Font *font, std::string fontFile, unsigned int fontSize )
	{
		accessMutex.Lock();
		curFont = font;
		curFontFile = fontFile;
		curFontSize = fontSize;
		accessMutex.Unlock();
		while ( curFont != NULL ) {
			Sleep(1);
		}
	}

	void processCurFont()
	{
		accessMutex.Lock();
		if ( curFont != NULL ) {
			dawn_debug_info( "loading font %s\n", curFontFile.c_str());
			processFilesDirectly = true;
			curFont->open( curFontFile, curFontSize );
			processFilesDirectly = false;
			curFont = NULL;
		}
		accessMutex.Unlock();
	}

	void init()
	{
		dawn_debug_info("Starting initialization");
		progressString = "Initializing Editor";
		Editor.LoadTextures();
		progressString = "Initializing GUI";
		progress = 0.025;
		GUI.LoadTextures();
		GUI.SetPlayer(&character);
		progressString = "Initializing Character Screen";
		progress = 0.05;
		characterInfoScreen = std::auto_ptr<CharacterInfoScreen>( new CharacterInfoScreen( &character ) );
		characterInfoScreen->LoadTextures();
		progressString = "Initializing Inventory Screen";
		progress = 0.075;
		inventoryScreen = std::auto_ptr<InventoryScreen>( new InventoryScreen( &character ) );
		inventoryScreen->loadTextures();
		progressString = "Initializing Action Bar";
		progress = 0.1;
		actionBar = std::auto_ptr<ActionBar>( new ActionBar( &character ) );
		actionBar->loadTextures();
		progressString = "Initializing Spellbook";
		progress = 0.125;
		spellbook = std::auto_ptr<Spellbook>( new Spellbook( &character ) );
		spellbook->loadTextures();
		progressString = "Initializing Buff Display";
		progress = 0.15;
		buffWindow = std::auto_ptr<BuffWindow>( new BuffWindow( &character ) );
		progressString = "Initializing Quest Screen";
		progress = 0.175;
		questWindow = std::auto_ptr<QuestWindow>( new QuestWindow );
		progressString = "Initializing Menu Screen";
		progress = 0.2;
		optionsWindow = std::auto_ptr<OptionsWindow>( new OptionsWindow );

		/// testing the shop, should not be initialized like this!!!
		shopWindow = std::auto_ptr<Shop>( new Shop( &character, NULL /* was dynamic_cast<CNPC*>( &character ) [=NULL] */ ) );

		dawn_debug_info("Loading the game data files and objects");
		progressString = "Loading Spell Data";
		progress = 0.225;
        LuaFunctions::executeLuaFile("data/spells.lua");
        progressString = "Loading Item Data";
        progress = 0.375;
        LuaFunctions::executeLuaFile("data/itemdatabase.lua");
        progressString = "Loading Mob Data";
        progress = 0.525;
		LuaFunctions::executeLuaFile("data/mobdata.all");
		dawn_debug_info("Loading completed");

		progressString = "Loading Character Data";
		progress = 0.7;
		CZone *newZone = new CZone();
		newZone->LoadZone("data/zone1");
		ActivityType::ActivityType activity = ActivityType::Walking;
		character.setNumMoveTexturesPerDirection( activity, 8 );
		for ( size_t curIndex=0; curIndex<8; ++curIndex ) {
			std::ostringstream ostr;
			ostr << "000" << curIndex;
			std::string numberString = ostr.str();
			character.setMoveTexture( activity, N, curIndex, std::string("data/character/swordsman/walking n").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, NE, curIndex, std::string("data/character/swordsman/walking ne").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, E, curIndex, std::string("data/character/swordsman/walking e").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, SE, curIndex, std::string("data/character/swordsman/walking se").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, S, curIndex, std::string("data/character/swordsman/walking s").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, SW, curIndex, std::string("data/character/swordsman/walking sw").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, W, curIndex, std::string("data/character/swordsman/walking w").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, NW, curIndex, std::string("data/character/swordsman/walking nw").append(numberString).append(".tga" ) );
		}
		activity = ActivityType::Attacking;
		character.setNumMoveTexturesPerDirection( activity, 13 );
		for ( size_t curIndex=0; curIndex<13; ++curIndex ) {
			std::ostringstream ostr;
			if ( curIndex < 10 )
				ostr << "000" << curIndex;
			else
				ostr << "00" << curIndex;

			std::string numberString = ostr.str();
			character.setMoveTexture( activity, N, curIndex, std::string("data/character/swordsman/attacking n").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, NE, curIndex, std::string("data/character/swordsman/attacking ne").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, E, curIndex, std::string("data/character/swordsman/attacking e").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, SE, curIndex, std::string("data/character/swordsman/attacking se").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, S, curIndex, std::string("data/character/swordsman/attacking s").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, SW, curIndex, std::string("data/character/swordsman/attacking sw").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, W, curIndex, std::string("data/character/swordsman/attacking w").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, NW, curIndex, std::string("data/character/swordsman/attacking nw").append(numberString).append(".tga" ) );
		}
		activity = ActivityType::Casting;
		character.setNumMoveTexturesPerDirection( activity, 13 );
		for ( size_t curIndex=0; curIndex<13; ++curIndex ) {
			std::ostringstream ostr;
			if ( curIndex < 10 )
				ostr << "000" << curIndex;
			else
				ostr << "00" << curIndex;

			std::string numberString = ostr.str();
			character.setMoveTexture( activity, N, curIndex, std::string("data/character/swordsman/attacking n").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, NE, curIndex, std::string("data/character/swordsman/attacking ne").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, E, curIndex, std::string("data/character/swordsman/attacking e").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, SE, curIndex, std::string("data/character/swordsman/attacking se").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, S, curIndex, std::string("data/character/swordsman/attacking s").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, SW, curIndex, std::string("data/character/swordsman/attacking sw").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, W, curIndex, std::string("data/character/swordsman/attacking w").append(numberString).append(".tga" ) );
			character.setMoveTexture( activity, NW, curIndex, std::string("data/character/swordsman/attacking nw").append(numberString).append(".tga" ) );
		}




		character.setMoveTexture( ActivityType::Walking, STOP, 0, "data/character/swordsman/walking s0000.tga" );
		character.setBoundingBox( 18, 20, 64, 64 );
		character.setUseBoundingBox( true );
		character.Init(dawn_configuration::screenWidth/2,dawn_configuration::screenHeight/2);
		character.setActiveGUI( &GUI );
		character.setMaxHealth(400);
		character.setMaxMana(250);
		character.setMaxFatigue(100);
		character.setStrength(15);
		character.setVitality(15);
		character.setDexterity(20);
		character.setWisdom(10);
		character.setIntellect(10);
		character.setManaRegen(2);
		character.setFatigueRegen( 5 );
		character.giveCoins( 576 );

		dawn_debug_info("Character completed");

		progressString = "Initializing load/save functions";
		progress = 0.92;
		LuaFunctions::executeLuaFile("data/loadsave.lua");
		progressString = "Loading Game Init Data";
		progress = 0.95;
		LuaFunctions::executeLuaFile("data/gameinit.lua");

		// initialize random number generator
		progressString = "Initializing Random Number Generator";
		progress = 0.99;
		srand( time( 0 ) );

		accessMutex.Lock();
		finished = true;
		accessMutex.Unlock();
	}

	virtual BOOL OnTask()
	{
		if ( started )
			return true;
		started = true;
		init();
		return true;
	}
};

void processTextureInOpenGLThread( CTexture *texture, std::string textureFile, int textureIndex )
{
	curTextureProcessor->setCurrentTextureToProcess( texture, textureFile, textureIndex );
}

void processFontInOpenGLThread( GLFT_Font *font, const std::string &filename, unsigned int size )
{
	curTextureProcessor->setCurrentFontToProcess( font, filename, size );
}

bool dawn_init(int argc, char** argv)
{
		if(!HandleCommandLineAurguments(argc, argv))
			return false;

		std::string sdlVideoCenteredParam( "SDL_VIDEO_CENTERED=1" );
		putenv( const_cast<char*>(sdlVideoCenteredParam.c_str()) );

		dawn_debug_info("Initializing...");
		dawn_init_signal_handlers();
		dawn_debug_info("Checking if the game data exists");

		if(!utils::file_exists("data/spells.lua"))
			dawn_debug_fatal("The LUA script \"spells.lua\", "
				"Could not be found");
		if(!utils::file_exists("data/mobdata.all"))
			dawn_debug_fatal("The LUA script \"mobdata.all\", "
				"Could not be found");
		if(!utils::file_exists("data/itemdatabase.lua"))
			dawn_debug_fatal("The LUA script \"itemdatabase.lua\", "
				"Could not be found");
		if(!utils::file_exists("data/gameinit.lua"))
			dawn_debug_fatal("The LUA script \"gameinit.lua\", "
					"Could not be found");
		if(!utils::file_exists("data/verdana.ttf"))
			dawn_debug_fatal("Font  \"verdana.ttf\", "
				"Could not be found");

		if (SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0)
			dawn_debug_fatal("Unable to init SDL: %s", SDL_GetError());

		atexit(SDL_Quit);

		if (dawn_configuration::fullscreenenabled)
			screen = SDL_SetVideoMode(dawn_configuration::screenWidth,
			                          dawn_configuration::screenHeight, dawn_configuration::bpp,
			                          SDL_OPENGL | SDL_FULLSCREEN);
		else
			screen = SDL_SetVideoMode(dawn_configuration::screenWidth,
			                          dawn_configuration::screenHeight, dawn_configuration::bpp,
			                          SDL_OPENGL);


		if ( !screen )
			dawn_debug_fatal("Unable to set resolution");

		glEnable( GL_TEXTURE_2D );

		glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
		glViewport( 0, 0, dawn_configuration::screenWidth, dawn_configuration::screenHeight );

		glClear( GL_COLOR_BUFFER_BIT );

		glMatrixMode( GL_PROJECTION );
		glLoadIdentity(); // reset view to 0,0

		glOrtho(0.0f, dawn_configuration::screenWidth, 0.0f, dawn_configuration::screenHeight, -1.0f, 1.0f);
		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();  // reset view to 0,0

		glEnable( GL_BLEND ); // enable blending
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);	// Turn Depth Testing Off

		std::auto_ptr<LoadingScreen> loadingScreen( new LoadingScreen() );
		DawnInitObject obj;
		processFilesDirectly = false;
		curTextureProcessor = &obj;
		do {
			obj.Event();
			Sleep( 10 );
		} while ( ! obj.started );

		while ( ! obj.isFinished() ) {
			obj.processCurTexture();
			obj.processCurFont();
			loadingScreen->setCurrentText( obj.getCurrentText() );
			loadingScreen->setProgress( obj.getProgress() );
			glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			loadingScreen->draw();
			SDL_GL_SwapBuffers();
		}

		processFilesDirectly = true;
		curTextureProcessor = NULL;

		// initialize fonts where needed
		fpsFont = FontCache::getFontFromCache("data/verdana.ttf", 12);
		message.initFonts();
		Editor.initFonts();
		characterInfoScreen->initFonts();
        actionBar->initFonts();
        GUI.initFonts();

		return true;
}

void setQuitGame()
{
	done = 1;
}

void game_loop()
{

	// TODO: Break this down into subroutines

	Uint32 lastTicks = SDL_GetTicks();
	Uint32 curTicks  = lastTicks;
	Uint32 ticksDiff = 0;
	Uint8 *keys;
	std::pair<int,int> mouseDownXY;
    done = 0;

    focus.setFocus(&character);

	while (!done) {

		if (Editor.isEnabled()) {
			Editor.HandleKeys();

			lastTicks = SDL_GetTicks();
			curTicks  = lastTicks;
			ticksDiff = 0;
		} else {
			SDL_Event event;

			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT)  {
					done = 1;
				}

				if (event.type == SDL_MOUSEBUTTONDOWN) {
                    mouseDownXY = std::pair<int,int>( mouseX, mouseY );

                    bool clickedInFrame = false;

                    // iterate through all our active frames and click on them if mouse is over.
                    for ( int curFrame = activeFrames.size()-1; curFrame >= 0; --curFrame )
                    {
                        if ( activeFrames[ curFrame ]->isMouseOnFrame( mouseX, mouseY ) )
                        {
                            // check if mouse is over closebutton (if any) and then we try and close the frame
                            if ( activeFrames[ curFrame ]->isMouseOnCloseButton( mouseX, mouseY ) == true )
                            {
                                activeFrames[ curFrame ]->toggle();
                                clickedInFrame = true;
                                break;
                            }

                            // check to see if mouse is over titlebar, then we try to move the frame.
                            if ( activeFrames[ curFrame ]->isMouseOnTitlebar( mouseX, mouseY ) == true )
                            {
                                activeFrames[ curFrame ]->moveFrame( mouseX, mouseY );
                                activeFrames[ curFrame ]->setOnTop();
                                clickedInFrame = true;
                                break;
                            }

                            activeFrames[ curFrame ]->clicked( mouseX, mouseY, event.button.button );
                            activeFrames[ curFrame ]->setOnTop();
                            clickedInFrame = true;
                            break;
                        }
                    }

                    // looks like we clicked without finding any frame to click on. this could mean that we want to interact with the background in some way. let's try that.
                    if ( clickedInFrame == false )
                    {
                        actionBar->clicked( mouseX, mouseY );
                        if ( shopWindow->hasFloatingSelection() )
                        {
                            shopWindow->clicked( mouseX, mouseY, event.button.button );
                        }

                        if ( inventoryScreen->hasFloatingSelection() )
                        {
                            inventoryScreen->clicked( mouseX, mouseY, event.button.button );
                        }

                        if ( spellbook->hasFloatingSpell() )
                        {
                            spellbook->clicked( mouseX, mouseY, event.button.button );
                        }

                        switch (event.button.button) {
                            case SDL_BUTTON_LEFT:
                            {
                                CZone *curZone = Globals::getCurrentZone();
                                curZone->getGroundLoot()->searchForItems( world_x + mouseX, world_y + mouseY );

                                if ( inventoryScreen->isVisible() )
                                {
                                    InventoryItem *floatingSelection = curZone->getGroundLoot()->getFloatingSelection( world_x + mouseX, world_y + mouseY );
                                    if ( floatingSelection != NULL )
                                    {
                                        inventoryScreen->setFloatingSelection( floatingSelection );
                                    }
                                }

                                // search for new target
                                std::vector<CNPC*> zoneNPCs = curZone->getNPCs();
                                for (unsigned int x=0; x<zoneNPCs.size(); x++) {
                                    CNPC *curNPC = zoneNPCs[x];
                                    if ( curNPC->CheckMouseOver(mouseX+world_x,mouseY+world_y) ) {
                                        if ( ! curNPC->getAttitude() == Attitude::FRIENDLY ) {
                                            character.setTarget( curNPC );
                                            break;
                                        }
                                    }
                                }
                            }
                            break;

                            case SDL_BUTTON_RIGHT:
                            {
                                // look for interactionpoints when right-clicking.
                                std::vector<InteractionPoint*> zoneInteractionPoints = Globals::getCurrentZone()->getInteractionPoints();
                                for ( size_t curInteractionNr=0; curInteractionNr < zoneInteractionPoints.size(); ++curInteractionNr ) {
                                    InteractionPoint *curInteraction = zoneInteractionPoints[ curInteractionNr ];
                                    if ( curInteraction->isMouseOver( mouseX, mouseY ) ) {
                                        curInteraction->startInteraction( character.getXPos(), character.getYPos() );
                                        break;
                                    }
                                }
                            }
                            break;
                        }
                    }
                }

                if (event.type == SDL_MOUSEMOTION)
                {
                    mouseX = event.motion.x;
                    mouseY = dawn_configuration::screenHeight - event.motion.y - 1;

                    if ( sqrt(pow(mouseDownXY.first-mouseX,2) + pow(mouseDownXY.second-mouseY,2)) > 25 )
                    {
                        actionBar->dragSpell();
                    }

                    for ( int curFrame = activeFrames.size()-1; curFrame >= 0; --curFrame )
                    {
                        if ( activeFrames[ curFrame ]->isMovingFrame() == true )
                        {
                            activeFrames[ curFrame ]->moveFrame( mouseX, mouseY );
                            break;
                        }
                    }
                }

                if (event.type == SDL_MOUSEBUTTONUP)
                {
                    actionBar->executeSpellQueue();

                    for ( int curFrame = activeFrames.size()-1; curFrame >= 0; --curFrame )
                    {
                        if ( activeFrames[ curFrame ]->isMovingFrame() == true )
                        {
                            activeFrames[ curFrame ]->stopMovingFrame( mouseX, mouseY );
                            break;
                        }
                    }
                }
            }

            // close and possibly delete closed windows
            for ( size_t curTextWindowNr=0; curTextWindowNr<allTextWindows.size(); ++curTextWindowNr ) {
                TextWindow *curTextWindow = allTextWindows[ curTextWindowNr ];
                if ( curTextWindow->canBeClosed() ) {
                    curTextWindow->close();
                    curTextWindow->toggle();
                    if ( curTextWindow->destroyAfterClose() )
                    {
                        delete curTextWindow;
                        allTextWindows.erase( allTextWindows.begin() + curTextWindowNr );
                    }
                }
            }

            keys = SDL_GetKeyState(NULL);

            curTicks  = SDL_GetTicks();
            ticksDiff = curTicks - lastTicks;
            lastTicks = curTicks;

            character.giveMovePoints( ticksDiff );
            character.Move();
            character.regenerateLifeManaFatigue( ticksDiff );


            std::vector<CNPC*> zoneNPCs = Globals::getCurrentZone()->getNPCs();
            for (unsigned int x=0; x<zoneNPCs.size(); x++) {
                CNPC *curNPC = zoneNPCs[x];
                if ( curNPC->isAlive() ) {
                    curNPC->giveMovePoints( ticksDiff );
                    curNPC->Move();
                }
                curNPC->Respawn();
                curNPC->Wander();
            }

            // making sure our target is still alive, if not well set our target to NULL.
            if (character.getTarget()) {
                if ( !character.getTarget()->isAlive() )
                    character.setTarget(NULL);
            }

            for (size_t curActiveSpellNr=0; curActiveSpellNr < activeSpellActions.size(); ++curActiveSpellNr ) {
                activeSpellActions[ curActiveSpellNr ]->inEffect();
            }

			std::vector<InteractionRegion*> interactionRegions = Globals::getCurrentZone()->getInteractionRegions();
			for ( size_t curInteractionRegionNr=0; curInteractionRegionNr<interactionRegions.size(); ++curInteractionRegionNr ) {
				InteractionRegion *curInteractionRegion = interactionRegions[ curInteractionRegionNr ];
				curInteractionRegion->interactWithPlayer( &character );
			}
			cleanupActiveSpellActions();
			Globals::getCurrentZone()->cleanupNPCList();
			Globals::getCurrentZone()->cleanupInteractionList();
			Globals::getCurrentZone()->cleanupInteractionRegionList();

            if (keys[SDLK_k]) { // kill all NPCs in the zone. testing purposes.
                std::vector<CNPC*> zoneNPCs = Globals::getCurrentZone()->getNPCs();
                for (unsigned int x=0; x<zoneNPCs.size(); x++) {
                    if ( zoneNPCs[x]->isAlive() ) {
                        zoneNPCs[x]->Die();
                    }
                }
            }

            if (event.key.keysym.sym == SDLK_PRINT && !KP_screenshot)
            {
                KP_screenshot = true;
                utils::takeScreenshot();
            }

            if (event.key.keysym.sym != SDLK_PRINT)
            {
                KP_screenshot = false;
            }

            if (keys[SDLK_l] && !Editor.KP_toggle_editor) {
                Editor.setEditZone( Globals::getCurrentZone() );
                Editor.setEnabled( true );
                Editor.initFocus( &focus );
                Editor.KP_toggle_editor = true;
            }

            if (!keys[SDLK_l]) {
                Editor.KP_toggle_editor = false;
            }

            if (keys[SDLK_TAB] && !KP_select_next) {
                KP_select_next = true;
                bool FoundNewTarget = false;
                std::vector <CNPC*> NPClist;
                // select next npc on screen
                std::vector<CNPC*> zoneNPCs = Globals::getCurrentZone()->getNPCs();
                for ( size_t curNPCNr = 0; curNPCNr < zoneNPCs.size(); ++curNPCNr ) {
                    // if NPC is in on screen (might be changed to line of sight or something)
                    // this makes a list of all visible NPCs, easier to select next target this way.
                    CNPC *curNPC = zoneNPCs[curNPCNr];
                    if ( DrawingHelpers::isRectOnScreen( curNPC->x_pos, 1, curNPC->y_pos, 1 )
                            && curNPC->isAlive() ) {
                        NPClist.push_back(curNPC);
                    }
                }
                // selects next target in the list, if target = NULL, set target to first NPC on the visible list.
                for ( size_t curNPC = 0; curNPC < NPClist.size(); ++curNPC ) {
                    if (!character.getTarget()) {
                        character.setTarget(NPClist[0]);
                    }

                    if ( character.getTarget() == NPClist[curNPC] ) {
                        if ( curNPC+1 == NPClist.size() ) {
                            character.setTarget(NPClist[0]);
                        } else {
                            character.setTarget(NPClist[curNPC+1]);
                        }
                        FoundNewTarget = true;
                        break;
                    }
                }

                if ( !FoundNewTarget && NPClist.size() > 0) {
                    character.setTarget(NPClist[0]);
                }
            }

            if (keys[SDLK_LALT])
            {
                Globals::getCurrentZone()->getGroundLoot()->enableTooltips();
            }

            if (!keys[SDLK_LALT])
            {
                Globals::getCurrentZone()->getGroundLoot()->disableTooltips();
            }

            if (!keys[SDLK_TAB]) {
                KP_select_next = false;
            }

            if (keys[SDLK_ESCAPE] && !KP_toggle_showOptionsWindow ) {
                KP_toggle_showOptionsWindow = true;
                optionsWindow->toggle();
            }

            if ( !keys[SDLK_ESCAPE] ) {
                KP_toggle_showOptionsWindow = false;
            }

            if ( keys[SDLK_c] && !KP_toggle_showCharacterInfo ) {
                KP_toggle_showCharacterInfo = true;
                characterInfoScreen->toggle();
            }

            if ( !keys[SDLK_c] ) {
                KP_toggle_showCharacterInfo = false;
            }

            if ( keys[SDLK_b] && !KP_toggle_showSpellbook ) {
                KP_toggle_showSpellbook = true;
                spellbook->toggle();
            }

            if ( !keys[SDLK_b] ) {
                KP_toggle_showSpellbook = false;
            }

            if ( keys[SDLK_i] && !KP_toggle_showInventory ) {
                KP_toggle_showInventory = true;
                inventoryScreen->toggle();
            }

            if ( !keys[SDLK_i] ) {
                KP_toggle_showInventory = false;
            }

            if ( keys[SDLK_q] && !KP_toggle_showQuestWindow ) {
                KP_toggle_showQuestWindow = true;
                questWindow->toggle();
            }

            if ( !keys[SDLK_q] ) {
                KP_toggle_showQuestWindow = false;
            }

            actionBar->handleKeys();

            if (keys[SDLK_5] && !KP_interrupt) {
                KP_interrupt = true;
                character.CastingInterrupted();
            }

            if (!keys[SDLK_5]) {
                KP_interrupt = false;
            }
        }
        DrawScene();
        focus.updateFocus();
	}
}

int main(int argc, char* argv[])
{
	dawn_configuration::logfile = "dawn-log.log";
	dawn_configuration::debug_stdout = true;
	dawn_configuration::debug_fileout = true;
	dawn_configuration::show_info_messages = true;
	dawn_configuration::show_warn_messages = true;

	if(dawn_init(argc, argv))
		game_loop();

	return 0;
}
