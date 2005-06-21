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
#include "graphics.h"
#include "gui/gui.h"
#include "gui/textfield.h"
#include "gui/minimap.h"
#include "gui/chargedialog.h"
#include "gui/itemcontainer.h"
#include "gui/trade.h"
#include "gui/buddywindow.h"
#include "gui/help.h"
#include "main.h"
#include "being.h"
#include "floor_item.h"

char itemCurrenyQ[10] = "0";
int map_x, map_y, camera_x, camera_y;
char npc_text[1000] = "";
char statsString2[255] = "n/a";
char skill_points[10] = "";
bool show_skill_dialog = false;
bool show_skill_list_dialog = false;
char npc_button[10] = "Close";

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
NewSkillDialog *newSkillWindow;
StatsWindow *statsWindow;
Setup* setupWindow;
Minimap *minimap;
EquipmentWindow *equipmentWindow;
ChargeDialog *chargeDialog;
TradeWindow *tradeWindow;
BuddyWindow *buddyWindow;
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

    if (being->action == WALK)
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

    if (being->action == WALK)
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
            std::string(homeDir) + std::string("chatlog.txt"));
    statusWindow = new StatusWindow();
    buyDialog = new BuyDialog();
    sellDialog = new SellDialog();
    buySellDialog = new BuySellDialog();
    inventoryWindow = new InventoryWindow();
    npcTextDialog = new NpcTextDialog();
    npcListDialog = new NpcListDialog();
    skillDialog = new SkillDialog();
    newSkillWindow = new NewSkillDialog();
    statsWindow = new StatsWindow();
    setupWindow = new Setup();
    minimap = new Minimap();
    equipmentWindow = new EquipmentWindow();
    chargeDialog = new ChargeDialog();
    tradeWindow = new TradeWindow();
    buddyWindow = new BuddyWindow();
    helpWindow = new HelpWindow();
    popupMenu = new PopupMenu();

    // Initialize window posisitons
    chatWindow->setPosition(0, screen->h - chatWindow->getHeight());
    statusWindow->setPosition(screen->w - statusWindow->getWidth() - 5, 5);
    inventoryWindow->setPosition(screen->w - statusWindow->getWidth() -
            inventoryWindow->getWidth() - 10, 5);
    statsWindow->setPosition(
            screen->w - 5 - statsWindow->getWidth(),
            statusWindow->getHeight() + 20);
    chargeDialog->setPosition(
            screen->w - 5 - chargeDialog->getWidth(),
            screen->h - chargeDialog->getHeight() - 15);
    tradeWindow->setPosition(screen->w - statusWindow->getWidth() -
            tradeWindow->getWidth() - 10,
            inventoryWindow->getY() + inventoryWindow->getHeight());
    buddyWindow->setPosition(10,
            minimap->getHeight() + 30);
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
    newSkillWindow->setVisible(false);
    statsWindow->setVisible(false);
    setupWindow->setVisible(false);
    equipmentWindow->setVisible(false);
    chargeDialog->setVisible(false);
    tradeWindow->setVisible(false);
    buddyWindow->setVisible(false);
    helpWindow->setVisible(false);
    popupMenu->setVisible(false);

    // Do not focus any text field
    gui->focusNone();

    // Load the sprite sets
    ResourceManager *resman = ResourceManager::getInstance();
    Image *npcbmp = resman->getImage(
            "graphics/sprites/npcs.png");
    Image *emotionbmp = resman->getImage(
            "graphics/sprites/emotions.png");
    Image *weaponbitmap = resman->getImage(
            "graphics/sprites/weapons.png");
    Image *itembitmap = resman->getImage(
            "graphics/sprites/items.png");

    if (!npcbmp) logger->error("Unable to load npcs.png");
    if (!emotionbmp) logger->error("Unable to load emotions.png");
    if (!weaponbitmap) logger->error("Unable to load weapons.png");
    if (!itembitmap) logger->error("Unable to load items.png");

    npcset = new Spriteset(npcbmp, 50, 80);
    emotionset = new Spriteset(emotionbmp, 19, 19);
    weaponset = new Spriteset(weaponbitmap, 160, 120);
    itemset = new Spriteset(itembitmap, 32, 32);
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
    delete newSkillWindow;
    delete tradeWindow;
    delete buddyWindow;
    delete helpWindow;
    delete popupMenu;

    // Delete sprite sets
    //delete monsterset;
    delete npcset;
    delete emotionset;
    delete weaponset;
    delete itemset;
}

Map *Engine::getCurrentMap()
{
    return mCurrentMap;
}

void Engine::setCurrentMap(Map *newMap)
{
    mCurrentMap = newMap;
    minimap->setMap(mCurrentMap);
    std::string musicFile = newMap->getProperty("music");
    
    if(musicFile!="") {
        musicFile = std::string(TMW_DATADIR) + "data/music/" + musicFile;
        sound.playMusic(musicFile.c_str(), -1);
    }
}

void Engine::logic()
{
    // Update beings
    std::list<Being*>::iterator beingIterator = beings.begin();
    while (beingIterator != beings.end())
    {
        Being *being = (*beingIterator);

        being->logic();

        if (being->action == MONSTER_DEAD && being->frame >= 20)
        {
            if (autoTarget == being) {
                autoTarget = NULL;
            }
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

    camera_x = map_x / 32;
    camera_y = map_y / 32;

    int offset_x = map_x & 31;
    int offset_y = map_y & 31;

    sort();

    frame++;

    // Draw tiles below nodes
    if (mCurrentMap != NULL)
    {
        mCurrentMap->draw(guiGraphics, map_x, map_y, 0);
        mCurrentMap->draw(guiGraphics, map_x, map_y, 1);
    }

    // Draw items
    std::list<FloorItem*>::iterator floorItemIterator = floorItems.begin();
    while (floorItemIterator != floorItems.end())
    {
        FloorItem *floorItem = (*floorItemIterator);
        unsigned short x = floorItem->x;
        unsigned short y = floorItem->y;
        int sx = x - camera_x;
        int sy = y - camera_y;
        int absx = sx * 32 - offset_x;
        int absy = sy * 32 - offset_y;
        if (itemDb->getItemInfo(floorItem->id)->getImage() > 0) {
            itemset->spriteset[itemDb->getItemInfo(
                    floorItem->id)->getImage() - 1]->draw(screen,
                    absx, absy);
        }

        floorItemIterator++;
    }

    // Draw nodes
    std::list<Being*>::iterator beingIterator = beings.begin();
    while (beingIterator != beings.end())
    {
        Being *being = (*beingIterator);

        unsigned short x = being->x;
        unsigned short y = being->y;
        unsigned char dir = being->direction / 2;
        int sx = x - camera_x;
        int sy = y - camera_y;

#ifdef DEBUG
        guiGraphics->setColor(gcn::Color(0, 0, 255));
        guiGraphics->drawRectangle(gcn::Rectangle(sx * 32, sy * 32, 32, 32));
#endif

        if (being->isNpc()) { // Draw a NPC
            npcset->spriteset[being->job - 100]->draw(screen,
                    sx * 32 - 8 - offset_x,
                    sy * 32 - 52 - offset_y);
        }
        else if ((being->job < 10) && (being->name != "")) { // Draw a player
            being->text_x = sx * 32 + get_x_offset(being) - offset_x;
            being->text_y = sy * 32 + get_y_offset(being) - offset_y;

            if (being->action == SIT || being->action == DEAD) {
                being->frame = 0;
            }

            int pf = being->frame + being->action;

            if (being->action == ATTACK) {
                if (being->weapon > 0)
                    pf += 4 * (being->weapon - 1);
            }

            playerset->spriteset[pf + 16 * dir]->draw(screen,
                    being->text_x - 16, being->text_y - 32);

            if (being->weapon != 0 && being->action == ATTACK) {
                weaponset->spriteset[16 * (being->weapon - 1) + 4 * being->frame + dir]->draw(screen,
                    being->text_x - 64, being->text_y - 80);
            }

            if (being->getHairColor() <= NR_HAIR_COLORS) {
                int hf = being->getHairColor() - 1 + 10 * (dir + 4 *
                        (being->getHairStyle() - 1));

                hairset->spriteset[hf]->draw(screen,
                        being->text_x - 2 + 2 * hairtable[pf][dir][0],
                        being->text_y - 50 + 2 * hairtable[pf][dir][1]);
            }

            if (being->emotion != 0) {
                emotionset->spriteset[being->emotion - 1]->draw(screen,
                        sx * 32 + 5 + get_x_offset(being) - offset_x,
                        sy * 32 - 65 + get_y_offset(being) - offset_y);
            }

            graphics->drawText(being->name,
                being->text_x + 15, being->text_y + 30,
                gcn::Graphics::CENTER);
        }
        else if (being->job == 45) { // Draw a warp
        }
        else { // Draw a monster
            if (being->frame >= 4)
                being->frame = 3;

            being->text_x = sx * 32 - 42 + get_x_offset(being) - offset_x;
            being->text_y = sy * 32 - 65 + get_y_offset(being) - offset_y;

            int mf = being->frame + being->action;

            if (being->action == MONSTER_DEAD) {
                monsterset[being->job - 1002]->spriteset[dir + 4 * MONSTER_DEAD]->draw(screen,
                        being->text_x + 30, being->text_y + 40);

                if (autoTarget == being) {
                    autoTarget = NULL;
                }
            }
            else {
                monsterset[being->job-1002]->spriteset[dir + 4 * mf]->draw(
                        screen, being->text_x + 30, being->text_y + 40);
            }

            if (being->action != STAND) {
                being->frame =
                    (get_elapsed_time(being->walk_time) * 4) / (being->speed);

                if (being->frame >= 4 && being->action != MONSTER_DEAD) {
                    being->nextStep();
                }
            }
        }

        beingIterator++;

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
                mouseX / 32 + camera_x, mouseY / 32 + camera_y);

        while (!debugPath.empty())
        {
            PATH_NODE node = debugPath.front();
            debugPath.pop_front();

            int squareX = (node.x - camera_x) * 32 - offset_x + 12;
            int squareY = (node.y - camera_y) * 32 - offset_y + 12;
            guiGraphics->setColor(gcn::Color(255, 0, 0));
            guiGraphics->fillRectangle(gcn::Rectangle(squareX, squareY, 8, 8));

            MetaTile *tile = mCurrentMap->getMetaTile(node.x, node.y);

            std::stringstream cost;
            cost << tile->Gcost;
            guiGraphics->drawText(cost.str(), squareX + 4, squareY + 12,
                    gcn::Graphics::CENTER);
        }
    }

    // Draw player speech
    beingIterator = beings.begin();
    while (beingIterator != beings.end()) {
        Being *being = (*beingIterator);

        being->drawSpeech(guiGraphics);

        beingIterator++;
    }

    if (statsWindow->isVisible()) {
        statsWindow->update();
    }
    if (statusWindow->isVisible()) {
        statusWindow->update();
    }

    gui->draw();

    std::stringstream debugStream;
    debugStream << "[" << fps << " fps] " <<
        (mouseX / 32 + camera_x) << ", " << (mouseY / 32 + camera_y);

    if (mCurrentMap != NULL)
    {
        debugStream
            << " [music: " << mCurrentMap->getProperty("music") << "]"
            << " [minimap: " << mCurrentMap->getProperty("minimap") << "]";
    }

    debugInfo->setCaption(debugStream.str());
    debugInfo->adjustSize();
}
