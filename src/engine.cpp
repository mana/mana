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

#include <guichan.hpp>
#include "game.h"
#include "main.h"
#include "playerinfo.h"
#include "engine.h"
#include "log.h"
#include "being.h"
#include "floor_item.h"
#include "graphics.h"
#include "gui/gui.h"
#include "gui/minimap.h"
#include "gui/chargedialog.h"
#include "gui/trade.h"
#include "gui/chat.h"
#include "gui/status.h"
#include "gui/buy.h"
#include "gui/sell.h"
#include "gui/buysell.h"
#include "gui/inventorywindow.h"
#include "gui/npc_text.h"
#include "gui/npc.h"
#include "gui/stats.h"
#include "gui/setup.h"
#include "gui/equipmentwindow.h"
#include "gui/popupmenu.h"
//#include "gui/buddywindow.h"
#include "gui/help.h"
#include "resources/resourcemanager.h"
#include "resources/itemmanager.h"

extern Being* autoTarget;
extern Graphics* graphics;

char itemCurrenyQ[10] = "0";
int map_x, map_y, camera_x, camera_y;

gcn::Label *debugInfo;

ChatWindow *chatWindow;
StatusWindow *statusWindow;
BuyDialog *buyDialog;
SellDialog *sellDialog;
BuySellDialog *buySellDialog;
InventoryWindow *inventoryWindow;
NpcListDialog *npcListDialog;
NpcTextDialog *npcTextDialog;
SkillDialog *skillDialog;
//NewSkillDialog *newSkillWindow;
StatsWindow *statsWindow;
Setup* setupWindow;
Minimap *minimap;
EquipmentWindow *equipmentWindow;
ChargeDialog *chargeDialog;
TradeWindow *tradeWindow;
//BuddyWindow *buddyWindow;
HelpWindow *helpWindow;
PopupMenu *popupMenu;
std::map<int, Spriteset*> monsterset;

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
        if (direction != NORTH && direction != SOUTH)
        {
            offset = (get_elapsed_time(being->walk_time) * 32) / being->speed;
            if (offset > 32) offset = 32;

            if (direction == WEST || direction == NW || direction == SW) {
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
        if (direction != EAST && direction != WEST)
        {
            offset = (get_elapsed_time(being->walk_time) * 32) / being->speed;
            if (offset > 32) offset = 32;

            if (direction == NORTH || direction == NW || direction == NE) {
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
    guiTop->add(debugInfo);

    // Create dialogs
    chatWindow = new ChatWindow(
            config.getValue("homeDir", "") + std::string("/chatlog.txt"));
    statusWindow = new StatusWindow();
    buyDialog = new BuyDialog();
    sellDialog = new SellDialog();
    buySellDialog = new BuySellDialog();
    inventoryWindow = new InventoryWindow();
    npcTextDialog = new NpcTextDialog();
    npcListDialog = new NpcListDialog();
    skillDialog = new SkillDialog();
    //newSkillWindow = new NewSkillDialog();
    statsWindow = new StatsWindow();
    setupWindow = new Setup();
    minimap = new Minimap();
    equipmentWindow = new EquipmentWindow();
    chargeDialog = new ChargeDialog();
    tradeWindow = new TradeWindow();
    //buddyWindow = new BuddyWindow();
    helpWindow = new HelpWindow();
    popupMenu = new PopupMenu();

    // Initialize window posisitons
    int screenW = guiGraphics->getWidth();
    int screenH = guiGraphics->getHeight();

    chatWindow->setPosition(0, screenH - chatWindow->getHeight());
    statusWindow->setPosition(screenW - statusWindow->getWidth() - 5, 5);
    inventoryWindow->setPosition(screenW - statusWindow->getWidth() -
            inventoryWindow->getWidth() - 10, 5);
    statsWindow->setPosition(
            screenW - 5 - statsWindow->getWidth(),
            statusWindow->getHeight() + 20);
    chargeDialog->setPosition(
            screenW - 5 - chargeDialog->getWidth(),
            screenH - chargeDialog->getHeight() - 15);
    tradeWindow->setPosition(screenW - statusWindow->getWidth() -
            tradeWindow->getWidth() - 10,
            inventoryWindow->getY() + inventoryWindow->getHeight());
    /*buddyWindow->setPosition(10,
            minimap->getHeight() + 30);*/
    equipmentWindow->setPosition(5,140);

    // Set initial window visibility
    chatWindow->setVisible(true);
    statusWindow->setVisible(true);
    buyDialog->setVisible(false);
    sellDialog->setVisible(false);
    buySellDialog->setVisible(false);
    inventoryWindow->setVisible(false);
    npcTextDialog->setVisible(false);
    npcListDialog->setVisible(false);
    skillDialog->setVisible(false);
    //newSkillWindow->setVisible(false);
    statsWindow->setVisible(false);
    setupWindow->setVisible(false);
    equipmentWindow->setVisible(false);
    chargeDialog->setVisible(false);
    tradeWindow->setVisible(false);
    //buddyWindow->setVisible(false);
    helpWindow->setVisible(false);
    popupMenu->setVisible(false);

    // Do not focus any text field
    gui->focusNone();

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
}

Engine::~Engine()
{
    // Delete windows
    delete chatWindow;
    delete statusWindow;
    delete buyDialog;
    delete sellDialog;
    delete buySellDialog;
    delete inventoryWindow;
    delete npcListDialog;
    delete npcTextDialog;
    delete skillDialog;
    delete statsWindow;
    delete setupWindow;
    delete minimap;
    delete equipmentWindow;
    delete chargeDialog;
    //delete newSkillWindow;
    delete tradeWindow;
    //delete buddyWindow;
    delete helpWindow;
    delete popupMenu;

    // Delete sprite sets
    //delete monsterset;
    delete npcset;
    delete emotionset;
    delete weaponset;
    delete itemset;

    attackTarget->decRef();
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

    map_x = (player_node->x - 13) * 32 + get_x_offset(player_node);
    map_y = (player_node->y -  9) * 32 + get_y_offset(player_node);

    if (map_x < 0) {
        map_x = 0;
    }
    if (map_y < 0) {
        map_y = 0;
    }

    if (mCurrentMap) {
        if (map_x > (mCurrentMap->getWidth() - 13) * 32) {
            map_x = (mCurrentMap->getWidth() - 13) * 32;
        }
        if (map_y > (mCurrentMap->getHeight() - 9) * 32) {
            map_y = (mCurrentMap->getHeight() - 9) * 32;
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
        mCurrentMap->draw(guiGraphics, map_x, map_y, 0);
        mCurrentMap->draw(guiGraphics, map_x, map_y, 1);
    }

    // Draw items
    for (std::list<FloorItem*>::iterator i = floorItems.begin(); i != floorItems.end(); i++)
    {
        FloorItem *floorItem = (*i);
        if (itemDb->getItemInfo(floorItem->id)->getImage() > 0) {
            Image *image = itemset->spriteset[itemDb->getItemInfo(
                    floorItem->id)->getImage() - 1];

            guiGraphics->drawImage(image,
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
        guiGraphics->setColor(gcn::Color(0, 0, 255));
        guiGraphics->drawRectangle(gcn::Rectangle(x & ~31, y & ~31, 32, 32));
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

                guiGraphics->drawImage(playerset->spriteset[frame + 16 * dir],
                        being->text_x - 16, being->text_y - 32);

                //if (being->action == ATTACK)
                //{
                //    std::cout << being->name << " " << being->getWeapon() << std::endl;
                //}
                if (being->getWeapon() != 0 && being->action == Being::ATTACK) {
                    Image *image = weaponset->spriteset[
                        16 * (being->getWeapon() - 1) + 4 * being->frame + dir];

                    guiGraphics->drawImage(image,
                            being->text_x - 64, being->text_y - 80);
                }

                if (being->getHairColor() <= NR_HAIR_COLORS) {
                    int hf = being->getHairColor() - 1 + 10 * (dir + 4 *
                            (being->getHairStyle() - 1));

                    guiGraphics->drawImage(hairset->spriteset[hf],
                            being->text_x - 2 + 2 * hairtable[frame][dir][0],
                            being->text_y - 50 + 2 * hairtable[frame][dir][1]);
                }

                if (being->emotion != 0) {
                    guiGraphics->drawImage(
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
                guiGraphics->drawImage(npcset->spriteset[being->job - 100],
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
                    guiGraphics->drawImage(
                            monsterset[being->job - 1002]->spriteset[dir + 4 * Being::MONSTER_DEAD],
                            being->text_x + 30, being->text_y + 40);
                }
                else {
                    guiGraphics->drawImage(
                            monsterset[being->job-1002]->spriteset[dir + 4 * frame],
                            being->text_x + 30, being->text_y + 40);

                    if (being->x == mouseTileX && being->y == mouseTileY)
                    {
                        guiGraphics->drawImage(attackTarget,
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
        mCurrentMap->draw(guiGraphics, map_x, map_y, 2);
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
            guiGraphics->setColor(gcn::Color(255, 0, 0));
            if (useOpenGL) {
#ifdef USE_OPENGL
                dynamic_cast<gcn::OpenGLGraphics*>(graphics)->fillRectangle(gcn::Rectangle(squareX, squareY, 8, 8));
#endif
            }
            else {
                dynamic_cast<gcn::SDLGraphics*>(graphics)->fillRectangle(gcn::Rectangle(squareX, squareY, 8, 8));
            }

            MetaTile *tile = mCurrentMap->getMetaTile(node.x, node.y);

            std::stringstream cost;
            cost << tile->Gcost;
            guiGraphics->drawText(cost.str(), squareX + 4, squareY + 12,
                    gcn::Graphics::CENTER);
        }
    }

    // Draw player speech
    for (std::list<Being*>::iterator i = beings.begin(); i != beings.end(); i++)
    {
        Being *being = (*i);

        being->drawSpeech(guiGraphics);
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
