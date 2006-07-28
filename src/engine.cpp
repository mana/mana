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

#include "net/messageout.h"
#include "net/protocol.h"

#include "resources/itemmanager.h"
#include "resources/mapreader.h"
#include "resources/resourcemanager.h"
#include "resources/spriteset.h"

#include "utils/dtor.h"
#include "utils/tostring.h"

extern Minimap *minimap;

char itemCurrenyQ[10] = "0";
int camera_x, camera_y;

ItemManager *itemDb;          /**< Item database object */

Spriteset *itemset;
Spriteset *emotionset;
Spriteset *npcset;
std::vector<Spriteset *> weaponset;

Engine::Engine(Network *network):
    mShowDebugPath(false),
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
    itemset = resman->getSpriteset("graphics/sprites/items.png", 32, 32);

    if (!npcset) logger->error("Unable to load NPC spriteset!");
    if (!emotionset) logger->error("Unable to load emotions spriteset!");
    if (!itemset) logger->error("Unable to load item spriteset!");

    // Initialize item manager
    itemDb = new ItemManager();
}

Engine::~Engine()
{
    // Delete sprite sets
    npcset->decRef();
    emotionset->decRef();
    itemset->decRef();

    std::vector<Spriteset *>::iterator iter;
    for (iter = weaponset.begin(); iter != weaponset.end(); ++iter)
    {
        (*iter)->decRef();
    }
    weaponset.clear();

    delete itemDb;
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
    int midTileX = graphics->getWidth() / 32 / 2;
    int midTileY = graphics->getHeight() / 32 / 2;

    int map_x = (player_node->mX - midTileX) * 32 + player_node->getXOffset();
    int map_y = (player_node->mY - midTileY) * 32 + player_node->getYOffset();

    if (mCurrentMap) {
        if (map_x < 0) {
            map_x = 0;
        }
        if (map_y < 0) {
            map_y = 0;
        }
        if (map_x > (mCurrentMap->getWidth() - midTileX) * 32) {
            map_x = (mCurrentMap->getWidth() - midTileX) * 32;
        }
        if (map_y > (mCurrentMap->getHeight() - midTileY) * 32) {
            map_y = (mCurrentMap->getHeight() - midTileY) * 32;
        }
    }

    camera_x = map_x / 32;
    camera_y = map_y / 32;

    // Draw tiles and sprites
    if (mCurrentMap != NULL)
    {
        mCurrentMap->draw(graphics, map_x, map_y, 0);
        mCurrentMap->draw(graphics, map_x, map_y, 1);
        mCurrentMap->draw(graphics, map_x, map_y, 2);
    }

    // Find a path from the player to the mouse, and draw it. This is for debug
    // purposes.
    if (mShowDebugPath && mCurrentMap != NULL)
    {
        // Get the current mouse position
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        int mouseTileX = mouseX / 32 + camera_x;
        int mouseTileY = mouseY / 32 + camera_y;

        Path debugPath = mCurrentMap->findPath(
                player_node->mX, player_node->mY,
                mouseTileX, mouseTileY);

        for (PathIterator i = debugPath.begin(); i != debugPath.end(); i++)
        {
            int squareX = i->x * 32 - map_x + 12;
            int squareY = i->y * 32 - map_y + 12;

            graphics->setColor(gcn::Color(255, 0, 0));
            graphics->fillRectangle(gcn::Rectangle(squareX, squareY, 8, 8));
            graphics->drawText(
                    toString(mCurrentMap->getMetaTile(i->x, i->y)->Gcost),
                    squareX + 4, squareY + 12, gcn::Graphics::CENTER);
        }
    }

    // Draw player nickname, speech, and emotion sprite as needed
    Beings *beings = beingManager->getAll();
    for (BeingIterator i = beings->begin(); i != beings->end(); i++)
    {
        (*i)->drawSpeech(graphics, -map_x, -map_y);
        (*i)->drawName(graphics, -map_x, -map_y);
        (*i)->drawEmotion(graphics, -map_x, -map_y);
    }

    // Draw target marker if needed
    Being *target;
    if ((target = player_node->getTarget()))
    {
        graphics->setFont(speechFont);
        graphics->setColor(gcn::Color(255, 255, 255));
        int dy = (target->getType() == Being::PLAYER) ? 90 : 52;

        graphics->drawText("[TARGET]", target->getPixelX() - map_x + 15,
                target->getPixelY() - map_y  - dy, gcn::Graphics::CENTER);
    }

    gui->draw();
}
