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
#include "gui/item_amount.h"
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

gcn::TextField *chatInput;
gcn::Label *debugInfo;

Window *chatWindow;
ChatBox *chatBox;
StatusWindow *statusWindow;
BuyDialog *buyDialog;
SellDialog *sellDialog;
BuySellDialog *buySellDialog;
InventoryWindow *inventoryWindow;
ItemAmountWindow *itemAmountWindow;
NpcListDialog *npcListDialog;
NpcTextDialog *npcTextDialog;
SkillDialog *skillDialog;
NewSkillDialog *newSkillWindow;
StatsWindow *statsWindow;
Setup* setupWindow;
Minimap *minimap;
EquipmentWindow *equipmentWindow;
ChargeDialog *chargeDialog;

void ChatListener::action(const std::string& eventId)
{
    if (eventId == "chatinput") {
        std::string message = chatInput->getText();

        if (message.length() > 0) {
            chatBox->chat_send(char_info[0].name, message.c_str());
            chatInput->setText("");
        }
    }
}

char hairtable[16][4][2] = {
    // S(x,y)  W(x,y)   N(x,y)   E(x,y)
    { { 0, 0}, {-1, 2}, {-1, 2}, {0, 2} }, // STAND
    { { 0, 2}, {-2, 3}, {-1, 2}, {1, 3} }, // WALK 1st frame
    { { 0, 3}, {-2, 4}, {-1, 3}, {1, 4} }, // WALK 2nd frame
    { { 0, 1}, {-2, 2}, {-1, 2}, {1, 2} }, // WALK 3rd frame
    { { 0, 2}, {-2, 3}, {-1, 2}, {1, 3} }, // WALK 4th frame
    { { 0, 1}, {1, 2}, {-1, 3}, {-2, 2} }, // ATTACK 1st frame
    { { 0, 1}, {-1, 2}, {-1, 3}, {0, 2} }, // ATTACK 2nd frame
    { { 0, 2}, {-4, 3}, {0, 4}, {3, 3}  }, // ATTACK 3rd frame
    { { 0, 2}, {-4, 3}, {0, 4}, {3, 3}  }, // ATTACK 4th frame
    { { 0, 0}, {-1, 2}, {-1, 2}, {-1, 2} }, // BOW_ATTACK 1st frame
    { { 0, 0}, {-1, 2}, {-1, 2}, {-1, 2} }, // BOW_ATTACK 2nd frame
    { { 0, 0}, {-1, 2}, {-1, 2}, {-1, 2}  }, // BOW_ATTACK 3rd frame
    { { 0, 0}, {-1, 2}, {-1, 2}, {-1, 2}  }, // BOW_ATTACK 4th frame
    { { 0, 4}, {-1, 6}, {-1, 6}, {0, 6} }, // SIT
    { { 0, 0}, {0, 0}, {0, 0}, {0, 0} }, // ?? HIT
    { { 0, 16}, {-1, 6}, {-1, 6}, {0, 6} } // DEAD
};

int get_x_offset(Being *being) {
    int offset = 0;
    char direction = being->direction;
    if (being->action == WALK) {
        if (direction != NORTH && direction != SOUTH) {
            offset = being->frame + 1;
            if (offset == 5) offset = 0;
            offset *= 8;
            if (direction == WEST || direction == NW || direction == SW) {
                offset = -offset;
                offset += 32;
            } else offset -= 32;
        }
    }
    return offset;
}

int get_y_offset(Being *being) {
    int offset = 0;
    char direction = being->direction;
    if (being->action == WALK) {
        if (direction != EAST && direction != WEST) {
            offset = being->frame + 1;
            if (offset == 5) offset = 0;
            offset *= 8;
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

Engine::Engine()
{
    // Initializes GUI
    chatWindow = new Window("Chat");
    chatWindow->setSize(600, 100);
    
    chatInput = new TextField();
    //chatInput->setPosition(chatInput->getBorderSize(),
    //        screen->h - chatInput->getHeight() -
    //        chatInput->getBorderSize());
    chatInput->setPosition(chatInput->getBorderSize(),
            chatWindow->getY() + 85);
    
    chatInput->setWidth(592 - 2 * chatInput->getBorderSize());

    ChatListener *chatListener = new ChatListener();
    chatInput->setEventId("chatinput");
    chatInput->addActionListener(chatListener);

    debugInfo = new gcn::Label();

    chatBox = new ChatBox("./docs/chatlog.txt", 20);
    chatBox->setSize(592, 100);
    chatBox->setPosition(0, chatInput->getY() - 1 - chatBox->getHeight());

    chatWindow->add(chatBox);
    //chatWindow->add(debugInfo);
    chatWindow->add(chatInput);
    
    chatWindow->setPosition(0, screen->h-12 - chatInput->getHeight() - chatBox->getHeight());
    
    guiTop->add(chatWindow);

    // Create dialogs

    statusWindow = new StatusWindow();
    statusWindow->setPosition(screen->w - statusWindow->getWidth() - 5, 5);

    buyDialog = new BuyDialog();
    buyDialog->setVisible(false);

    sellDialog = new SellDialog();
    sellDialog->setVisible(false);

    buySellDialog = new BuySellDialog();
    buySellDialog->setVisible(false);

    inventoryWindow = new InventoryWindow();
    inventoryWindow->setVisible(false);
    inventoryWindow->setPosition(screen->w - statusWindow->getWidth() -
            inventoryWindow->getWidth() - 10, 5);

    itemAmountWindow = new ItemAmountWindow();
    itemAmountWindow->setVisible(false);
    itemAmountWindow->setPosition(screen->w - statusWindow->getWidth() - 
            inventoryWindow->getWidth() - 10, inventoryWindow->getHeight() + 
            10);

    npcTextDialog = new NpcTextDialog();
    npcTextDialog->setVisible(false);

    npcListDialog = new NpcListDialog();
    npcListDialog->setVisible(false);

    skillDialog = new SkillDialog();
    skillDialog->setVisible(false);

    newSkillWindow = new NewSkillDialog();
    newSkillWindow->setVisible(false);


    statsWindow = new StatsWindow();
    statsWindow->setVisible(false);
    statsWindow->setPosition(
            screen->w - 5 - statsWindow->getWidth(),
            statusWindow->getHeight() + 20);

    setupWindow = new Setup();
    setupWindow->setVisible(false);

    minimap = new Minimap();

    equipmentWindow = new EquipmentWindow();
    equipmentWindow->setVisible(false);
    
    chargeDialog = new ChargeDialog();
    chargeDialog->setVisible(true);
    chargeDialog->setPosition(
            screen->w - 5 - chargeDialog->getWidth(),
            screen->h - chargeDialog->getHeight() - 15);
    
    // Give focus to the chat input
    chatInput->requestFocus();

    // Load the sprite sets
    ResourceManager *resman = ResourceManager::getInstance();
    Image *npcbmp = resman->getImage(
            "core/graphics/sprites/npcs.png");
    Image *emotionbmp = resman->getImage(
            "core/graphics/sprites/emotions.png");
    Image *monsterbitmap = resman->getImage(
            "core/graphics/sprites/monsters.png");
    Image *weaponbitmap = resman->getImage(
            "core/graphics/sprites/weapons.png");
    Image *itembitmap = resman->getImage(
            "core/graphics/sprites/items.png", IMG_ALPHA);

    if (!npcbmp) error("Unable to load npcs.png");
    if (!emotionbmp) error("Unable to load emotions.png");
    if (!monsterbitmap) error("Unable to load monsters.png");
    if (!weaponbitmap) error("Unable to load weapons.png");
    if (!itembitmap) error("Unable to load items.png");

    npcset = new Spriteset(npcbmp, 50, 80);
    emotionset = new Spriteset(emotionbmp, 19, 19);
    monsterset = new Spriteset(monsterbitmap, 60, 60);
    weaponset = new Spriteset(weaponbitmap, 160, 120);
    itemset = new Spriteset(itembitmap, 20, 20); 
}

Engine::~Engine()
{
    delete chatBox;
    delete statusWindow;
    delete buyDialog;
    delete sellDialog;
    delete buySellDialog;
    delete npcListDialog;
    delete npcTextDialog;
    delete skillDialog;
    delete statsWindow;
    delete setupWindow;
    delete minimap;
    delete equipmentWindow;
    delete newSkillWindow;
    delete itemAmountWindow;
	
    delete monsterset;
    delete npcset;
    delete emotionset;
    delete weaponset;
    delete itemset;
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
    if (tiledMap) {
        tiledMap->draw(guiGraphics, map_x, map_y, 0);
        tiledMap->draw(guiGraphics, map_x, map_y, 1);
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
        if (floorItem->id >= 501 && floorItem->id <= 1202) {
                itemset->spriteset[floorItem->id - 501]->draw(screen,
                         absx,
                         absy);
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

        if ((being->job >= 100) && (being->job <= 110)) { // Draw a NPC
            npcset->spriteset[4 * (being->job - 100) + dir]->draw(screen,
                    sx * 32 - 8 - offset_x,
                    sy * 32 - 52 - offset_y);
        }
        else if (being->job < 10) { // Draw a player
            being->text_x = sx * 32 + get_x_offset(being) - offset_x;
            being->text_y = sy * 32 + get_y_offset(being) - offset_y;

            if (being->action == SIT || being->action == DEAD) {
                being->frame = 0;
            }

            int pf = being->frame + being->action;

            if (being->action == ATTACK) {
                if(being->weapon > 0)
                    pf += 4 * (being->weapon - 1);
            }

            playerset->spriteset[4 * pf + dir]->draw(screen,
                    being->text_x - 64, being->text_y - 80);
            if (being->weapon != 0 && being->action == ATTACK) {
                weaponset->spriteset[4 * being->frame + dir]->draw(screen,
                    being->text_x - 64, being->text_y - 80);
            }

            if (being->hair_color <= 10) {
                int hf = being->hair_color - 1 + 10 * (dir + 4 *
                        (being->hair_style - 1));

                hairset->spriteset[hf]->draw(screen,
                        being->text_x - 2 + 2 * hairtable[pf][dir][0],
                        being->text_y - 50 + 2 * hairtable[pf][dir][1]);
            }

            if (being->emotion != 0) {
                emotionset->spriteset[being->emotion - 1]->draw(screen,
                        sx * 32 - 5 + get_x_offset(being) - offset_x,
                        sy * 32 - 45 + get_y_offset(being) - offset_y);
                being->emotion_time--;
                if (being->emotion_time == 0) {
                    being->emotion = 0;
                }
            }
            if (being->action != STAND && being->action != SIT
                    && being->action != DEAD) {
                being->frame =
                    (get_elapsed_time(being->walk_time) * 4) / (being->speed);

                if (being->frame >= 4) {
                    being->nextStep();
                }
            }
        }
        else if (being->job == 45) { // Draw a warp
        } else { // Draw a monster
            if (being->frame >= 4)
                being->frame = 3;

            being->text_x = sx * 32 - 42 + get_x_offset(being) - offset_x;
            being->text_y = sy * 32 - 65 + get_y_offset(being) - offset_y;

            int sprnum = dir + 4 * (being->job - 1002);
            int mf = being->frame + being->action;

            if (being->action == MONSTER_DEAD) {
                monsterset->spriteset[sprnum + 16 * MONSTER_DEAD]->draw(screen,
                        being->text_x + 30, being->text_y + 40);
            }
            else {
                monsterset->spriteset[sprnum + 16 * mf]->draw(screen,
                        being->text_x + 30, being->text_y + 40);
            }

            if (being->action != STAND) {
                being->frame =
                    (get_elapsed_time(being->walk_time) * 4) / (being->speed);

                if (being->frame >= 4 && being->action != MONSTER_DEAD) {
                    being->nextStep();
                }
            }
        }

        if (being->action == MONSTER_DEAD && being->frame >= 20) {
            delete being;
            beingIterator = beings.erase(beingIterator);
        }
        else {
            beingIterator++;
        }

        // nodes are ordered so if the next being y is > then the
        // last drawed for fringe layer, draw the missing lines
    }

    // Draw tiles below nodes
    if (tiledMap) {
        tiledMap->draw(guiGraphics, map_x, map_y, 2);
    }

    // Find a path from the player to the mouse, and draw it. This is for debug
    // purposes.
    if (displayPathToMouse)
    {
        std::list<PATH_NODE> debugPath = tiledMap->findPath(
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

            MetaTile *tile = tiledMap->getMetaTile(node.x, node.y);

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

        // Tick the beings (gives them a sense of time)
        being->tick();

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
        (mouseX / 32 + camera_x) << ", " << (mouseY / 32 + camera_y) << " "
        << player_node->weapon;
    debugInfo->setCaption(debugStream.str());
    debugInfo->adjustSize();
}
