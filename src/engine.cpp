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
#include "map.h"
#include "sound.h"

#include "gui/gui.h"
#include "gui/minimap.h"

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

Spriteset *emotionset;
Spriteset *npcset;
std::vector<Spriteset *> weaponset;

Engine::Engine():
    mShowDebugPath(false),
    mCurrentMap(NULL)
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

    // Initialize item manager
    itemDb = new ItemManager();
}

Engine::~Engine()
{
    // Delete sprite sets
    npcset->decRef();
    emotionset->decRef();

    std::for_each(weaponset.begin(), weaponset.end(),
            std::mem_fun(&Spriteset::decRef));
    weaponset.clear();

    delete itemDb;
}

void Engine::changeMap(const std::string &mapPath)
{
    // Clean up floor items
    floorItemManager->clear();

    beingManager->clear();

    // Store full map path in global var
    map_path = "maps/" + mapPath;

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
}

void Engine::logic()
{
    beingManager->logic();
    gui->logic();
}

void Engine::draw(Graphics *graphics)
{
    int midTileX = graphics->getWidth() / 2;
    int midTileY = graphics->getHeight() / 2;
    static int lastTick = tick_time;

    int player_x = player_node->mX - midTileX + player_node->getXOffset();
    int player_y = player_node->mY - midTileY + player_node->getYOffset();

    scrollLaziness = (int)config.getValue("ScrollLaziness", 32);
    scrollRadius = (int)config.getValue("ScrollRadius", 32);

    if (scrollLaziness < 1)
        scrollLaziness = 1; //avoids division by zero

    //apply lazy scrolling
    int nbTicks = get_elapsed_time(lastTick) / 10;
    lastTick += nbTicks;
    for (; nbTicks > 0; --nbTicks)
    {
        if (player_x > view_x + scrollRadius)
        {
            view_x += (player_x - view_x - scrollRadius) / scrollLaziness;
        }
        if (player_x < view_x - scrollRadius)
        {
            view_x += (player_x - view_x + scrollRadius) / scrollLaziness;
        }
        if (player_y > view_y + scrollRadius)
        {
            view_y += (player_y - view_y - scrollRadius) / scrollLaziness;
        }
        if (player_y < view_y - scrollRadius)
        {
            view_y += (player_y - view_y + scrollRadius) / scrollLaziness;
        }
    }

    //auto center when player is off screen
    if (        player_x - view_x > graphics->getWidth() / 2
            ||  view_x - player_x > graphics->getWidth() / 2
            ||  view_y - player_y > graphics->getHeight() / 2
            ||  player_y - view_y > graphics->getHeight() / 2
        )
    {
        view_x = player_x;
        view_y = player_y;
    };

    if (mCurrentMap) {
        if (view_x < 0) {
            view_x = 0;
        }
        if (view_y < 0) {
            view_y = 0;
        }
        if (view_x > mCurrentMap->getWidth() * 32 - midTileX) {
            view_x = mCurrentMap->getWidth() * 32 - midTileX;
        }
        if (view_y > mCurrentMap->getHeight() * 32 - midTileY) {
            view_y = mCurrentMap->getHeight() * 32 - midTileY;
        }
    }

    camera_x = (int)view_x;
    camera_y = (int)view_y;

    // Draw tiles and sprites
    if (mCurrentMap != NULL)
    {
        mCurrentMap->draw(graphics, camera_x, camera_y, 0);
        mCurrentMap->draw(graphics, camera_x, camera_y, 1);
        mCurrentMap->draw(graphics, camera_x, camera_y, 2);
        mCurrentMap->drawOverlay(   graphics,
                                    view_x,
                                    view_y,
                                    (int)config.getValue("OverlayDetail", 2)
                                );
    }
    else
    {
        // When no map is loaded, draw a replacement background
        graphics->setColor(gcn::Color(128, 128, 128));
        graphics->fillRectangle(gcn::Rectangle(0, 0,
                    graphics->getWidth(), graphics->getHeight()));
    }

    // Find a path from the player to the mouse, and draw it. This is for debug
    // purposes.
    if (mShowDebugPath && mCurrentMap != NULL)
    {
        // Get the current mouse position
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);

        int mouseTileX = (mouseX + camera_x) / 32;
        int mouseTileY = (mouseY + camera_y) / 32;

        Path debugPath = mCurrentMap->findPath(
                player_node->mX / 32, player_node->mY / 32,
                mouseTileX, mouseTileY);

        graphics->setColor(gcn::Color(255, 0, 0));
        for (PathIterator i = debugPath.begin(); i != debugPath.end(); i++)
        {
            int squareX = i->x * 32 - camera_x + 12;
            int squareY = i->y * 32 - camera_y + 12;

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
        (*i)->drawSpeech(graphics, -camera_x, -camera_y);
        (*i)->drawName(graphics, -camera_x, -camera_y);
        (*i)->drawEmotion(graphics, -camera_x, -camera_y);
    }

    // Draw target marker if needed
    Being *target;
    if ((target = player_node->getTarget()))
    {
        graphics->setFont(speechFont);
        graphics->setColor(gcn::Color(255, 255, 255));
        int dy = (target->getType() == Being::PLAYER) ? 90 : 52;

        graphics->drawText("[TARGET]", target->getPixelX() - camera_x + 15,
                target->getPixelY() - camera_y - dy, gcn::Graphics::CENTER);
    }

    gui->draw();
}
