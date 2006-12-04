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
#include "sound.h"

#include "gui/gui.h"
#include "gui/minimap.h"
#include "gui/viewport.h"

#include "net/messageout.h"
#include "net/protocol.h"

#include "resources/mapreader.h"
#include "resources/monsterdb.h"
#include "resources/resourcemanager.h"
#include "resources/spriteset.h"

#include "utils/dtor.h"
#include "utils/tostring.h"

extern Minimap *minimap;

char itemCurrenyQ[10] = "0";

Spriteset *emotionset;
Spriteset *npcset;
std::vector<Spriteset *> weaponset;

Engine::Engine(Network *network):
    mCurrentMap(NULL),
    mNetwork(network)
{
    // Load the sprite sets
    ResourceManager *resman = ResourceManager::getInstance();

    npcset = resman->getSpriteset("graphics/sprites/npcs.png", 50, 80);
    emotionset = resman->getSpriteset("graphics/sprites/emotions.png",
            30, 32);
    for (int i = 0; i < 2; i++)
    {
        Spriteset *tmp = ResourceManager::getInstance()->getSpriteset(
                "graphics/sprites/weapon" + toString(i) + ".png", 64, 64);
        if (!tmp) {
            logger->error("Unable to load weaponset");
        } else {
            weaponset.push_back(tmp);
        }
    }

    if (!npcset) logger->error("Unable to load NPC spriteset!");
    if (!emotionset) logger->error("Unable to load emotions spriteset!");
}

Engine::~Engine()
{
    // Delete sprite sets
    npcset->decRef();
    emotionset->decRef();

    std::for_each(weaponset.begin(), weaponset.end(),
            std::mem_fun(&Spriteset::decRef));
    weaponset.clear();
}

void Engine::changeMap(const std::string &mapPath)
{
    // Clean up floor items
    floorItemManager->clear();

    beingManager->clear();

    // Store full map path in global var
    map_path = "maps/" + mapPath.substr(0, mapPath.rfind(".")) + ".tmx.gz";

    // Attempt to load the new map
    Map *newMap = MapReader::readMap(map_path);

    if (!newMap) {
        logger->error("Could not find map file");
    }

    // Notify the minimap and beingManager about the map change
    Image *mapImage = NULL;
    if (newMap->hasProperty("minimap")) {
        ResourceManager *resman = ResourceManager::getInstance();
        mapImage = resman->getImage(newMap->getProperty("minimap"));
    }
    minimap->setMapImage(mapImage);
    beingManager->setMap(newMap);
    viewport->setMap(newMap);

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
    gui->logic();
}

void Engine::draw(Graphics *graphics)
{
    gui->draw();
}
