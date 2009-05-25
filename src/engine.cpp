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
#include "gui/okdialog.h"
#include "gui/viewport.h"

#include "net/maphandler.h"
#include "net/net.h"

#include "resources/mapreader.h"
#include "resources/monsterdb.h"
#include "resources/resourcemanager.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

Engine::Engine():
    mCurrentMap(0)
{
}

Engine::~Engine()
{
    delete mCurrentMap;
}

bool Engine::changeMap(const std::string &mapPath)
{
    // Clean up floor items, beings and particles
    floorItemManager->clear();
    beingManager->clear();

    // Close the popup menu on map change so that invalid options can't be
    // executed.
    viewport->closePopupMenu();

    // Unset the map of the player so that its particles are cleared before
    // being deleted in the next step
    if (player_node)
        player_node->setMap(0);

    particleEngine->clear();

    mMapName = mapPath;

    // Store full map path in global var
    map_path = "maps/" + mapPath + ".tmx";
    ResourceManager *resman = ResourceManager::getInstance();
    if (!resman->exists(map_path))
        map_path += ".gz";

    // Attempt to load the new map
    Map *newMap = MapReader::readMap(map_path);

    if (!newMap)
    {
        logger->log("Error while loading %s", map_path.c_str());
        new OkDialog(_("Could not load map"),
                     strprintf(_("Error while loading %s"), map_path.c_str()));
    }

    // Notify the minimap and beingManager about the map change
    minimap->setMap(newMap);
    beingManager->setMap(newMap);
    particleEngine->setMap(newMap);
    viewport->setMap(newMap);

    // Initialize map-based particle effects
    if (newMap)
        newMap->initializeParticleEffects(particleEngine);

    // Start playing new music file when necessary
    std::string oldMusic = mCurrentMap ? mCurrentMap->getMusicFile() : "";
    std::string newMusic = newMap ? newMap->getMusicFile() : "";
    if (newMusic != oldMusic)
        sound.playMusic(newMusic);

    delete mCurrentMap;
    mCurrentMap = newMap;

    Net::getMapHandler()->mapLoaded(mapPath);
    return true;
}

void Engine::logic()
{
    beingManager->logic();
    particleEngine->update();
    gui->logic();
}
