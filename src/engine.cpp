/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include "engine.h"

#include <list>

#include "being.h"
#include "beingmanager.h"
#include "configuration.h"
#include "flooritemmanager.h"
#include "game.h"
#include "graphics.h"
#include "main.h"
#include "localplayer.h"
#include "log.h"
#include "main.h"
#include "map.h"
#include "particle.h"
#include "sound.h"

#include "gui/gui.h"
#include "gui/minimap.h"
#include "gui/viewport.h"

#include "net/messageout.h"
#include "net/protocol.h"

#include "resources/mapreader.h"
#include "resources/monsterdb.h"
#include "resources/resourcemanager.h"

#include "utils/dtor.h"
#include "utils/tostring.h"

extern Minimap *minimap;

char itemCurrenyQ[10] = "0";

Engine::Engine(Network *network):
    mCurrentMap(NULL),
    mNetwork(network)
{
}

void Engine::changeMap(const std::string &mapPath)
{
    // Clean up floor items
    floorItemManager->clear();

    beingManager->clear();
    particleEngine->clear();

    // Store full map path in global var
    map_path = "maps/" + mapPath.substr(0, mapPath.rfind(".")) + ".tmx.gz";

    // Attempt to load the new map
    Map *newMap = MapReader::readMap(map_path);

    if (!newMap) {
        logger->error("Could not find map file");
    }

    // Notify the minimap and beingManager about the map change
    Image *mapImage = NULL;
    if (newMap->hasProperty("minimap"))
    {
        ResourceManager *resman = ResourceManager::getInstance();
        mapImage = resman->getImage(newMap->getProperty("minimap"));
    }
    minimap->setMapImage(mapImage);
    beingManager->setMap(newMap);
    particleEngine->setMap(newMap);
    viewport->setMap(newMap);

    // Initialize map-based particle effects
    newMap->initializeParticleEffects(particleEngine);

    // Start playing new music file when necessary
    std::string oldMusic = "";

    if (mCurrentMap) {
        oldMusic = mCurrentMap->getProperty("music");
        delete mCurrentMap;
    }

    std::string newMusic = newMap->getProperty("music");

    if (newMusic != oldMusic) {
        newMusic = std::string(TMW_DATADIR) + "data/music/" + newMusic;
        sound.playMusic(newMusic.c_str(), -1);
    }

    mCurrentMap = newMap;

    // Send "map loaded"
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_MAP_LOADED);
}

void Engine::logic()
{
    beingManager->logic();
    particleEngine->update();
    gui->logic();
}
