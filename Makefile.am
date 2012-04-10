# do not check for files README, TODO, etc
AUTOMAKE_OPTIONS = foreign

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/src/threadObject -I$(top_srcdir)/GLee -I/usr/include/freetype2 -I/usr/include/GL -I/usr/include/lua5.1
# LDADD = -lSDL -lSDL_image -lGL -llua5.1 -ltolua++5.1 -lfreetype -lglut -lGLU -lSDL_mixer


bin_PROGRAMS = dawn-rpg

# if configure is run with --with-testsuite we also need to build the testsuite program
if TESTSUITE
    bin_PROGRAMS += testsuite
    testsuite_SOURCES = tests/testsuite.cpp tests/testsuite.h
endif

#todo: Data should be installed in $(pkgdatadir) which is .../share/dawn
dawndatadir = $(bindir)/data


dawn_rpg_SOURCES = src/a_star.cpp \
	src/a_star.hpp \
	src/actionbar.cpp \
	src/actionbar.hpp \
	src/buffwindow.cpp \
	src/buffwindow.hpp \
	src/callindirection.cpp \
	src/callindirection.hpp \
	src/camerafocushandler.cpp \
	src/camerafocushandler.hpp \
	src/character.cpp \
	src/character.hpp \
	src/direction.hpp \
	src/drawinghelpers.hpp \
	src/editor.cpp \
	src/editor.hpp \
	src/characterinfoscreen.cpp \
	src/characterinfoscreen.hpp \
	src/interface.cpp \
	src/interface.hpp \
	src/luafunctions.cpp \
	src/luafunctions.hpp \
	src/luainterfacegenerated.cpp \
	src/luainterfacegenerated.hpp \
	src/luainterface.hpp \
	src/magic.hpp \
	src/magic.cpp \
	src/message.cpp \
	src/message.hpp \
	src/npc.cpp \
	src/npc.hpp \
	src/commonsounds.cpp \
	src/commonsounds.hpp \
	src/configurableframe.hpp \
	src/configurableframe.cpp \
	src/configuredframes.hpp \
	src/configuredframes.cpp \
	src/configuration.cpp \
	src/configuration.hpp \
	src/controlelement.hpp \
	src/controlelement.cpp \
	src/spell.cpp \
	src/spell.hpp \
	src/texture.cpp \
	src/texture.hpp \
	src/zone.cpp \
	src/zone.hpp \
	src/dawnstate.hpp \
	src/dawnstate.cpp \
	src/debug.cpp \
	src/debug.hpp \
	src/elements.cpp \
	src/elements.hpp \
	src/frames.hpp \
	src/frames.cpp \
	src/framesbase.cpp \
	src/framesbase.hpp \
	src/fontcache.cpp \
	src/fontcache.hpp \
	src/gameloophandler.hpp \
	src/gameloophandler.cpp \
	src/GLFT_Font.cpp \
	src/GLFT_Font.hpp \
	src/globals.cpp \
	src/globals.hpp \
	src/groundloot.cpp \
	src/groundloot.hpp \
	src/interactionpoint.cpp \
	src/interactionpoint.hpp \
	src/interactionregion.cpp \
	src/interactionregion.hpp \
	src/inventory.cpp \
	src/inventory.hpp \
	src/inventoryscreen.cpp \
	src/inventoryscreen.hpp \
	src/item.cpp \
	src/item.hpp \
	src/loadingmanager.hpp \
	src/loadingscreen.cpp \
	src/loadingscreen.hpp \
	src/logwindow.cpp \
	src/logwindow.hpp \
	src/main.cpp \
	src/musiccache.cpp \
	src/musiccache.hpp \
	src/optionswindow.cpp \
	src/optionswindow.hpp \
	src/player.cpp \
	src/player.hpp \
	src/quest.cpp \
	src/quest.hpp \
	src/questwindow.cpp \
	src/questwindow.hpp \
	src/random.cpp \
	src/random.hpp \
	src/resolution.cpp \
	src/resolution.hpp \
	src/shop.cpp \
	src/shop.hpp \
	src/soundengine.cpp \
	src/soundengine.h \
	src/spellbook.cpp \
	src/spellbook.hpp \
	src/stats.hpp \
	src/statssystem.cpp \
	src/statssystem.hpp \
	src/testinterface.cpp \
	src/testinterface.hpp \
	src/textureframe.hpp \
	src/textwindow.cpp \
	src/textwindow.hpp \
	src/tileset.cpp \
	src/tileset.hpp \
	src/timeconverterhelper.hpp \
	src/tooltip.cpp \
	src/tooltip.hpp \
	src/utils.cpp \
	src/utils.hpp \
	src/GLee/GLee.cpp \
	src/GLee/GLee.h \
	src/pnglite/pnglite.c \
	src/pnglite/pnglite.h \
	src/threadObject/EventClass.cpp \
	src/threadObject/EventClass.h \
	src/threadObject/MutexClass.cpp \
	src/threadObject/MutexClass.h \
	src/threadObject/Thread.cpp \
	src/threadObject/Thread.h

dawndata_DATA = data/spells.lua
