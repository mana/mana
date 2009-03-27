/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "beingmanager.h"
#include "engine.h"
#include "flooritemmanager.h"
#include "game.h"
#include "localplayer.h"
#include "log.h"
#include "map.h"
#include "particle.h"
#include "sound.h"

#include "gui/gui.h"
#include "gui/minimap.h"
#include "gui/viewport.h"

#ifdef EATHENA_SUPPORT
#include "net/messageout.h"
#include "net/ea/protocol.h"
#endif

#include "resources/mapreader.h"
#include "resources/monsterdb.h"
#include "resources/resourcemanager.h"

#include "utils/stringutils.h"

#ifdef TMWSERV_SUPPORT
Engine::Engine():
    mCurrentMap(NULL)
#else
Engine::Engine(Network *network):
    mCurrentMap(NULL),
    mNetwork(network)
#endif
{
}

Engine::~Engine()
{
    delete mCurrentMap;
}

void Engine::changeMap(const std::string &mapPath)
{
    // Clean up floor items, beings and particles
    floorItemManager->clear();
    beingManager->clear();
    particleEngine->clear();

    // Unset the map of the player so that its particles are cleared before
    // being deleted in the next step
    if (player_node)
        player_node->setMap(0);

    particleEngine->clear();

    mMapName = mapPath;

    // Store full map path in global var
#ifdef TMWSERV_SUPPORT
    map_path = "maps/" + mapPath + ".tmx";
#else
    map_path = "maps/" + mapPath.substr(0, mapPath.rfind(".")) + ".tmx";
#endif
    ResourceManager *resman = ResourceManager::getInstance();
    if (!resman->exists(map_path))
        map_path += ".gz";

    // Attempt to load the new map
    Map *newMap = MapReader::readMap(map_path);

    if (!newMap)
        logger->error("Could not find map file");

    // Notify the minimap and beingManager about the map change
    Image *mapImage = NULL;
    if (newMap->hasProperty("minimap"))
    {
        mapImage = resman->getImage(newMap->getProperty("minimap"));

        // Set the title for the Minimap
        if (newMap->hasProperty("mapname"))
             minimap->setCaption(newMap->getProperty("mapname"));
        else if (newMap->hasProperty("name"))
            minimap->setCaption(newMap->getProperty("name"));
        else
        {
             minimap->setCaption("Unknown");
             logger->log("WARNING: Map file '%s' defines a minimap image but "
                         "does not define a 'mapname' property",
                         map_path.c_str());
        }

        // How many pixels equal one tile. .5 (which is the TMW default) is
        // 2 tiles to a pixel, while 1 is 1 tile to 1 pixel
        if (newMap->hasProperty("minimapproportion"))
             minimap->setProportion(atof(
                      newMap->getProperty("minimapproportion").c_str()));
        else
             minimap->setProportion(0.5);
    }
    if (newMap->hasProperty("name"))
    {
        minimap->setCaption(newMap->getProperty("name"));
    } else {
        minimap->setCaption("Map");
    }
    minimap->setMapImage(mapImage);
    beingManager->setMap(newMap);
    particleEngine->setMap(newMap);
    viewport->setMap(newMap);

    // Initialize map-based particle effects
    newMap->initializeParticleEffects(particleEngine);

    // Start playing new music file when necessary
    std::string oldMusic = "";

    if (mCurrentMap)
    {
        oldMusic = mCurrentMap->getProperty("music");
        delete mCurrentMap;
    }

    std::string newMusic = newMap->getProperty("music");

    if (newMusic != oldMusic)
        sound.playMusic(newMusic, -1);

    mCurrentMap = newMap;

#ifdef EATHENA_SUPPORT
    // Send "map loaded"
    MessageOut outMsg(CMSG_MAP_LOADED);
#endif
}

void Engine::logic()
{
    beingManager->logic();
    particleEngine->update();
    if (mCurrentMap)
        mCurrentMap->update();
    gui->logic();
}
