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
#include <sstream>

#include "being.h"
#include "floor_item.h"
#include "game.h"
#include "graphics.h"
#include "log.h"
#include "main.h"
#include "map.h"
#include "sound.h"

#include "graphic/spriteset.h"

#include "gui/gui.h"
#include "gui/minimap.h"

#include "resources/image.h"
#include "resources/iteminfo.h"
#include "resources/itemmanager.h"
#include "resources/mapreader.h"
#include "resources/resourcemanager.h"

extern Being *autoTarget;
extern Graphics *graphics;
extern Minimap *minimap;
extern std::list<FloorItem*> floorItems;
extern int frame;

char itemCurrenyQ[10] = "0";
int camera_x, camera_y;

std::map<int, Spriteset*> monsterset;

ItemManager *itemDb;          /**< Item database object */

Spriteset *itemset;
Spriteset *emotionset;
Spriteset *npcset;
Spriteset *weaponset;


Engine::Engine():
    mCurrentMap(NULL)
{
    // Initializes GUI

    // Load the sprite sets
    ResourceManager *resman = ResourceManager::getInstance();
    Image *npcbmp = resman->getImage("graphics/sprites/npcs.png");
    Image *emotionbmp = resman->getImage("graphics/sprites/emotions.png");
    Image *weaponbitmap = resman->getImage("graphics/sprites/weapons.png");
    Image *itembitmap = resman->getImage("graphics/sprites/items.png");

    if (!npcbmp) logger->error("Unable to load npcs.png");
    if (!emotionbmp) logger->error("Unable to load emotions.png");
    if (!weaponbitmap) logger->error("Unable to load weapons.png");
    if (!itembitmap) logger->error("Unable to load items.png");

    npcset = new Spriteset(npcbmp, 50, 80);
    emotionset = new Spriteset(emotionbmp, 30, 32);
    weaponset = new Spriteset(weaponbitmap, 160, 120);
    itemset = new Spriteset(itembitmap, 32, 32);

    npcbmp->decRef();
    emotionbmp->decRef();
    weaponbitmap->decRef();
    itembitmap->decRef();

    attackTarget = resman->getImage("graphics/gui/attack_target.png");
    if (!attackTarget) logger->error("Unable to load attack_target.png");

    // Initialize item manager
    itemDb = new ItemManager();
}

Engine::~Engine()
{
    // Delete sprite sets
    std::map<int, Spriteset*>::iterator i;
    for (i = monsterset.begin(); i != monsterset.end(); i++)
    {
        delete (*i).second;
    }
    monsterset.clear();

    delete npcset;
    delete emotionset;
    delete weaponset;
    delete itemset;

    attackTarget->decRef();

    delete itemDb;
}

Map *Engine::getCurrentMap()
{
    return mCurrentMap;
}

void Engine::changeMap(const std::string &mapPath)
{
    // Clean up floor items
    empty_floor_items();

    // Remove the local player, so it is not deleted
    if (player_node != NULL)
    {
        beings.remove(player_node);
    }

    // Delete all beings (except the local player)
    std::list<Being*>::iterator i;
    for (i = beings.begin(); i != beings.end(); i++)
    {
        delete (*i);
    }
    beings.clear();

    // Attempt to load the new map
    Map *newMap = MapReader::readMap(mapPath);

    if (!newMap) {
        logger->error("Could not find map file");
    }

    // Re-add the local player node and transfer him to the newly loaded map
    if (player_node != NULL)
    {
        beings.push_back(player_node);
        player_node->setMap(newMap);
    }

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

    // Notify the minimap about the map change
    minimap->setMap(mCurrentMap);
}

void Engine::logic()
{
    // Update beings
    std::list<Being*>::iterator beingIterator = beings.begin();
    while (beingIterator != beings.end())
    {
        Being *being = (*beingIterator);

        being->logic();

        if (being->action == Being::MONSTER_DEAD && being->mFrame >= 20)
        {
            delete being;
            beingIterator = beings.erase(beingIterator);
        }
        else {
            beingIterator++;
        }
    }
}

void Engine::draw()
{
    // Get the current mouse position
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    int midTileX = graphics->getWidth() / 32 / 2;
    int midTileY = graphics->getHeight() / 32 / 2;

    int map_x = (player_node->x - midTileX) * 32 + player_node->getXOffset();
    int map_y = (player_node->y - midTileY) * 32 + player_node->getYOffset();

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
    int mouseTileX = mouseX / 32 + camera_x;
    int mouseTileY = mouseY / 32 + camera_y;

    frame++;

    // Draw tiles and sprites
    if (mCurrentMap != NULL)
    {
        mCurrentMap->draw(graphics, map_x, map_y, 0);
        mCurrentMap->draw(graphics, map_x, map_y, 1);
        mCurrentMap->draw(graphics, map_x, map_y, 2);
    }

    // Find a path from the player to the mouse, and draw it. This is for debug
    // purposes.
    if (displayPathToMouse && mCurrentMap != NULL)
    {
        std::list<PATH_NODE> debugPath = mCurrentMap->findPath(
                player_node->x, player_node->y,
                mouseTileX, mouseTileY);

        while (!debugPath.empty())
        {
            PATH_NODE node = debugPath.front();
            debugPath.pop_front();

            int squareX = node.x * 32 - map_x + 12;
            int squareY = node.y * 32 - map_y + 12;
            graphics->setColor(gcn::Color(255, 0, 0));
            graphics->fillRectangle(gcn::Rectangle(squareX, squareY, 8, 8));

            MetaTile *tile = mCurrentMap->getMetaTile(node.x, node.y);

            std::stringstream cost;
            cost << tile->Gcost;
            graphics->drawText(cost.str(), squareX + 4, squareY + 12,
                    gcn::Graphics::CENTER);
        }
    }

    // Draw player speech
    for (std::list<Being*>::iterator i = beings.begin(); i != beings.end(); i++)
    {
        (*i)->drawSpeech(graphics, -map_x, -map_y);
    }

    gui->draw();
}
