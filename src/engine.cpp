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

#include <sstream>

#include <guichan/imagefont.hpp>

#include <guichan/widgets/label.hpp>

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
#include "gui/stats.h"
#include "gui/status.h"
#include "gui/windowcontainer.h"

#include "resources/image.h"
#include "resources/iteminfo.h"
#include "resources/itemmanager.h"
#include "resources/resourcemanager.h"

extern Being *autoTarget;
extern Graphics *graphics;
extern Minimap *minimap;
extern Spriteset *hairset, *playerset;

char itemCurrenyQ[10] = "0";
int camera_x, camera_y;

gcn::Label *debugInfo;

std::map<int, Spriteset*> monsterset;

ItemManager *itemDb;          /**< Item database object */

char hairtable[16][4][2] = {
    // S(x,y)    W(x,y)   N(x,y)   E(x,y)
    { { 0,  0}, {-1, 2}, {-1, 2}, { 0, 2} }, // STAND
    { { 0,  2}, {-2, 3}, {-1, 2}, { 1, 3} }, // WALK 1st frame
    { { 0,  3}, {-2, 4}, {-1, 3}, { 1, 4} }, // WALK 2nd frame
    { { 0,  1}, {-2, 2}, {-1, 2}, { 1, 2} }, // WALK 3rd frame
    { { 0,  2}, {-2, 3}, {-1, 2}, { 1, 3} }, // WALK 4th frame
    { { 0,  1}, { 1, 2}, {-1, 3}, {-2, 2} }, // ATTACK 1st frame
    { { 0,  1}, {-1, 2}, {-1, 3}, { 0, 2} }, // ATTACK 2nd frame
    { { 0,  2}, {-4, 3}, { 0, 4}, { 3, 3} }, // ATTACK 3rd frame
    { { 0,  2}, {-4, 3}, { 0, 4}, { 3, 3} }, // ATTACK 4th frame
    { { 0,  0}, {-1, 2}, {-1, 2}, {-1, 2} }, // BOW_ATTACK 1st frame
    { { 0,  0}, {-1, 2}, {-1, 2}, {-1, 2} }, // BOW_ATTACK 2nd frame
    { { 0,  0}, {-1, 2}, {-1, 2}, {-1, 2} }, // BOW_ATTACK 3rd frame
    { { 0,  0}, {-1, 2}, {-1, 2}, {-1, 2} }, // BOW_ATTACK 4th frame
    { { 0,  4}, {-1, 6}, {-1, 6}, { 0, 6} }, // SIT
    { { 0,  0}, { 0, 0}, { 0, 0}, { 0, 0} }, // ?? HIT
    { { 0, 16}, {-1, 6}, {-1, 6}, { 0, 6} }  // DEAD
};


int get_x_offset(Being *being)
{
    int offset = 0;
    char direction = being->direction;

    if (being->action == Being::WALK)
    {
        if (direction != Being::NORTH && direction != Being::SOUTH)
        {
            offset = (get_elapsed_time(being->walk_time) * 32) / being->speed;
            if (offset > 32) offset = 32;

            if (direction == Being::WEST || direction == Being::NW ||
                    direction == Being::SW) {
                offset = -offset;
                offset += 32;
            }
            else {
                offset -= 32;
            }
        }
    }

    return offset;
}

int get_y_offset(Being *being)
{
    int offset = 0;
    char direction = being->direction;

    if (being->action == Being::WALK)
    {
        if (direction != Being::EAST && direction != Being::WEST)
        {
            offset = (get_elapsed_time(being->walk_time) * 32) / being->speed;
            if (offset > 32) offset = 32;

            if (direction == Being::NORTH || direction == Being::NW ||
                    direction == Being::NE) {
                offset = -offset;
                offset += 32;
            }
            else {
                offset -= 32;
            }
        }
    }

    return offset;
}

Engine::Engine():
    mCurrentMap(NULL)
{
    // Initializes GUI
    debugInfo = new gcn::Label();
    // Oh, come on guichan folks, how useful is it to have a single widget gui?
    // (Well, the BasicContainer interface isn't that much more useful... ;)
    dynamic_cast<WindowContainer*>(gui->getTop())->add(debugInfo);

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

    attackTarget = resman->getImage("graphics/gui/attack_target.png");
    if (!attackTarget) logger->error("Unable to load attack_target.png");

    // Initialize item manager
    itemDb = new ItemManager();

}

Engine::~Engine()
{
    // Delete sprite sets
    //delete monsterset;
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

void Engine::setCurrentMap(Map *newMap)
{
    std::string oldMusic = "";

    if (mCurrentMap) {
        oldMusic = mCurrentMap->getProperty("music");
    }

    std::string newMusic = newMap->getProperty("music");

    if (newMusic != oldMusic) {
        newMusic = std::string(TMW_DATADIR) + "data/music/" + newMusic;
        sound.playMusic(newMusic.c_str(), -1);
    }

    mCurrentMap = newMap;
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

        if (being->action == Being::MONSTER_DEAD && being->frame >= 20)
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

    int map_x = (player_node->x - midTileX) * 32 + get_x_offset(player_node);
    int map_y = (player_node->y -  midTileY) * 32 + get_y_offset(player_node);

    if (map_x < 0) {
        map_x = 0;
    }
    if (map_y < 0) {
        map_y = 0;
    }

    if (mCurrentMap) {
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

    sort();

    frame++;

    // Draw tiles below nodes
    if (mCurrentMap != NULL)
    {
        mCurrentMap->draw(graphics, map_x, map_y, 0);
        mCurrentMap->draw(graphics, map_x, map_y, 1);
    }

    // Draw items
    for (std::list<FloorItem*>::iterator i = floorItems.begin(); i != floorItems.end(); i++)
    {
        FloorItem *floorItem = (*i);
        if (itemDb->getItemInfo(floorItem->id)->getImage() > 0) {
            Image *image = itemset->spriteset[itemDb->getItemInfo(
                    floorItem->id)->getImage() - 1];

            graphics->drawImage(image,
                    floorItem->x * 32 - map_x, floorItem->y * 32 - map_y);
        }
    }

    // Draw nodes
    for (std::list<Being*>::iterator i = beings.begin(); i != beings.end(); i++)
    {
        Being *being = (*i);

        unsigned char dir = being->direction / 2;
        int x = being->x * 32 - map_x;
        int y = being->y * 32 - map_y;

#ifdef DEBUG
        graphics->setColor(gcn::Color(0, 0, 255));
        graphics->drawRectangle(gcn::Rectangle(x & ~31, y & ~31, 32, 32));
#endif
        int frame;
        switch (being->getType())
        {
            // Draw a player
            case Being::PLAYER:
                being->text_x = x + get_x_offset(being);
                being->text_y = y + get_y_offset(being);

                if (being->action == Being::SIT || being->action == Being::DEAD) {
                    being->frame = 0;
                }

                frame = being->frame + being->action;

                if (being->action == Being::ATTACK) {
                    if (being->getWeapon() > 0)
                        frame += 4 * (being->getWeapon() - 1);
                }

                graphics->drawImage(playerset->spriteset[frame + 16 * dir],
                        being->text_x - 16, being->text_y - 32);

                //if (being->action == ATTACK)
                //{
                //    std::cout << being->name << " " << being->getWeapon() << std::endl;
                //}
                if (being->getWeapon() != 0 && being->action == Being::ATTACK) {
                    Image *image = weaponset->spriteset[
                        16 * (being->getWeapon() - 1) + 4 * being->frame + dir];

                    graphics->drawImage(image,
                            being->text_x - 64, being->text_y - 80);
                }

                if (being->getHairColor() <= NR_HAIR_COLORS) {
                    int hf = being->getHairColor() - 1 + 10 * (dir + 4 *
                            (being->getHairStyle() - 1));

                    graphics->drawImage(hairset->spriteset[hf],
                            being->text_x - 2 + 2 * hairtable[frame][dir][0],
                            being->text_y - 50 + 2 * hairtable[frame][dir][1]);
                }

                if (being->emotion != 0) {
                    graphics->drawImage(
                            emotionset->spriteset[being->emotion - 1],
                            being->text_x + 3, being->text_y - 90);
                }

                graphics->setFont(speechFont);
                graphics->drawText(being->name,
                        being->text_x + 15, being->text_y + 30,
                        gcn::Graphics::CENTER);
                graphics->setFont(gui->getFont());
                break;

                // Draw a NPC
            case Being::NPC:
                graphics->drawImage(npcset->spriteset[being->job - 100],
                        x - 8, y - 52);
                break;

                // Draw a monster
            case Being::MONSTER:
                if (being->frame >= 4)
                {
                    being->frame = 3;
                }

                being->text_x = x - 42 + get_x_offset(being);
                being->text_y = y - 65 + get_y_offset(being);

                frame = being->frame + being->action;

                if (being->action == Being::MONSTER_DEAD) {
                    graphics->drawImage(
                            monsterset[being->job - 1002]->spriteset[dir + 4 * Being::MONSTER_DEAD],
                            being->text_x + 30, being->text_y + 40);
                }
                else {
                    graphics->drawImage(
                            monsterset[being->job-1002]->spriteset[dir + 4 * frame],
                            being->text_x + 30, being->text_y + 40);

                    if (being->x == mouseTileX && being->y == mouseTileY)
                    {
                        graphics->drawImage(attackTarget,
                                being->text_x + 30 + 16, being->text_y + 32);
                    }
                }

                if (being->action != Being::STAND) {
                    being->frame =
                        (get_elapsed_time(being->walk_time) * 4) / (being->speed);

                    if (being->frame >= 4 && being->action != Being::MONSTER_DEAD) {
                        being->nextStep();
                    }
                }
                break;

                /*
                // Draw a warp (job == 45)
            case Being::WARP:
                break;
                */

                // No idea how to draw this ;)
            default:
                break;
        }

        // nodes are ordered so if the next being y is > then the
        // last drawed for fringe layer, draw the missing lines
    }

    // Draw tiles below nodes
    if (mCurrentMap != NULL)
    {
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
        Being *being = (*i);

        being->drawSpeech(graphics);
    }

    if (autoTarget) {
        graphics->drawText("[TARGET]",
                autoTarget->text_x + 60,
                autoTarget->text_y,
                gcn::Graphics::CENTER);
    }

    if (statsWindow->isVisible()) {
        statsWindow->update();
    }
    if (statusWindow->isVisible()) {
        statusWindow->update();
    }

    std::stringstream debugStream;
    debugStream << "[" << fps << " fps] " << mouseTileX << ", " << mouseTileY;

    if (mCurrentMap != NULL)
    {
        debugStream
            << " [music: " << mCurrentMap->getProperty("music") << "]"
            << " [minimap: " << mCurrentMap->getProperty("minimap") << "]";
    }

    debugInfo->setCaption(debugStream.str());
    debugInfo->adjustSize();

    gui->draw();
}
