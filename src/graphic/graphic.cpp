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

#include "graphic.h"
#include "../gui/gui.h"
#include "../gui/textfield.h"
#include "../gui/status.h"
#include "../gui/minimap.h"
#include "../gui/equipment.h"
#include "../main.h"
#include "../being.h"

SDL_Surface *screen;

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

ChatBox *chatBox;
StatusWindow *statusWindow;
BuyDialog *buyDialog;
SellDialog *sellDialog;
BuySellDialog *buySellDialog;
InventoryWindow *inventoryWindow;
NpcListDialog *npcListDialog;
NpcTextDialog *npcTextDialog;
SkillDialog *skillDialog;
StatsWindow *statsWindow;
Setup* setupWindow;
Minimap *minimap;
EquipmentWindow *equipmentWindow;

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

void BuySellListener::action(const std::string& eventId)
{
    int actionId = -1;

    if (eventId == "buy") {
        actionId = 0;
    }
    else if (eventId == "sell") {
        actionId = 1;
    }

    if (actionId > -1) {
        WFIFOW(0) = net_w_value(0x00c5);
        WFIFOL(2) = net_l_value(current_npc);
        WFIFOB(6) = net_b_value(actionId);
        WFIFOSET(7);
    }

    buySellDialog->setVisible(false);
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


Graphics::Graphics():
    mouseCursor(NULL)
{
    setTarget(SDL_GetVideoSurface());

    // Load the mouse cursor
    ResourceManager *resman = ResourceManager::getInstance();
    mouseCursor = resman->getImage("core/graphics/gui/mouse.png", IMG_ALPHA);
    if (!mouseCursor) {
        error("Unable to load mouse cursor.");
    }

    // Hide the system mouse cursor
    SDL_ShowCursor(SDL_DISABLE);
}

Graphics::~Graphics() {
}

void Graphics::drawImageRect(
        int x, int y, int w, int h,
        Image *topLeft, Image *topRight,
        Image *bottomLeft, Image *bottomRight,
        Image *top, Image *right,
        Image *bottom, Image *left,
        Image *center)
{
    // Draw the center area
    center->drawPattern(screen,
            x + topLeft->getWidth(), y + topLeft->getHeight(),
            w - topLeft->getWidth() - topRight->getWidth(),
            h - topLeft->getHeight() - bottomLeft->getHeight());

    // Draw the sides
    top->drawPattern(screen,
            x + topLeft->getWidth(), y,
            w - topLeft->getWidth() - topRight->getWidth(), top->getHeight());
    bottom->drawPattern(screen,
            x + bottomLeft->getWidth(), y + h - bottom->getHeight(),
            w - bottomLeft->getWidth() - bottomRight->getWidth(),
            bottom->getHeight());
    left->drawPattern(screen,
            x, y + topLeft->getHeight(),
            left->getWidth(),
            h - topLeft->getHeight() - bottomLeft->getHeight());
    right->drawPattern(screen,
            x + w - right->getWidth(), y + topRight->getHeight(),
            right->getWidth(),
            h - topRight->getHeight() - bottomRight->getHeight());

    // Draw the corners
    topLeft->draw(screen, x, y);
    topRight->draw(screen, x + w - topRight->getWidth(), y);
    bottomLeft->draw(screen, x, y + h - bottomLeft->getHeight());
    bottomRight->draw(screen,
            x + w - bottomRight->getWidth(),
            y + h - bottomRight->getHeight());
}

void Graphics::drawImageRect(
        int x, int y, int w, int h,
        const ImageRect &imgRect)
{
    drawImageRect(x, y, w, h,
            imgRect.grid[0], imgRect.grid[2], imgRect.grid[6], imgRect.grid[8],
            imgRect.grid[1], imgRect.grid[5], imgRect.grid[7], imgRect.grid[3],
            imgRect.grid[4]);
}

void Graphics::updateScreen()
{
    // Draw mouse before flipping
    mouseCursor->draw(screen, mouseX - 5, mouseY - 2);

    SDL_Flip(screen);
}


Engine::Engine()
{
    // Initializes GUI
    chatInput = new TextField();
    chatInput->setPosition(chatInput->getBorderSize(),
        screen->h - chatInput->getHeight() - chatInput->getBorderSize() -1);
    chatInput->setWidth(592 - 2 * chatInput->getBorderSize());

    ChatListener *chatListener = new ChatListener();
    chatInput->setEventId("chatinput");
    chatInput->addActionListener(chatListener);

    debugInfo = new gcn::Label();

    chatBox = new ChatBox("./docs/chatlog.txt", 20);
    chatBox->setSize(592, 100);
    chatBox->setPosition(0, chatInput->getY() - 1 - chatBox->getHeight());

    guiTop->add(chatBox);
    guiTop->add(debugInfo);
    guiTop->add(chatInput);


    // Create dialogs

    statusWindow = new StatusWindow();
    statusWindow->setPosition(screen->w - statusWindow->getWidth() - 5, 5);

    buyDialog = new BuyDialog();
    buyDialog->setVisible(false);

    sellDialog = new SellDialog();
    sellDialog->setVisible(false);

    buySellDialog = new BuySellDialog(new BuySellListener());
    buySellDialog->setVisible(false);

    inventoryWindow = new InventoryWindow();
    inventoryWindow->setVisible(false);
    inventoryWindow->setPosition(screen->w - statusWindow->getWidth() - inventoryWindow->getWidth() - 10, 5);

    npcTextDialog = new NpcTextDialog();
    npcTextDialog->setVisible(false);

    npcListDialog = new NpcListDialog();
    npcListDialog->setVisible(false);

    skillDialog = new SkillDialog();
    skillDialog->setVisible(false);

    statsWindow = new StatsWindow();
    statsWindow->setVisible(false);
    statsWindow->setPosition(screen->w - 5 - statsWindow->getWidth(), statusWindow->getHeight() + 20);
    
    setupWindow = new Setup();
    setupWindow->setVisible(false);
    
    minimap = new Minimap();
    
    equipmentWindow = new EquipmentWindow();
    equipmentWindow->setVisible(true);

    // Give focus to the chat input
    chatInput->requestFocus();

    // Load the sprite sets
    ResourceManager *resman = ResourceManager::getInstance();
    Image *npcbmp = resman->getImage(
            "core/graphics/sprites/npcs.png");
    Image *emotionbmp = resman->getImage(
            "core/graphics/sprites/emotions.png");
    Image *tilesetbmp = resman->getImage(
            "core/graphics/tiles/desert.png");
    Image *monsterbitmap = resman->getImage(
            "core/graphics/sprites/monsters.png");

    if (!npcbmp) error("Unable to load npcs.png");
    if (!emotionbmp) error("Unable to load emotions.png");
    if (!tilesetbmp) error("Unable to load desert.png");
    if (!monsterbitmap) error("Unable to load monsters.png");

    npcset = new Spriteset(npcbmp, 50, 80);
    emotionset = new Spriteset(emotionbmp, 19, 19);
    tileset = new Spriteset(tilesetbmp, 32, 32);
    monsterset = new Spriteset(monsterbitmap, 60, 60);
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
    
    delete tileset;
    delete monsterset;
    delete npcset;
    delete emotionset;
}

void Engine::draw()
{
    map_x = (player_node->x - 13) * 32 +
        get_x_offset(player_node);
    map_y = (player_node->y - 9) * 32 +
        get_y_offset(player_node);

    camera_x = map_x >> 5;
    camera_y = map_y >> 5;

    int offset_x = map_x & 31;
    int offset_y = map_y & 31;

    sort();

    frame++;

    // Draw tiles below nodes
    for (int j = 0; j < 20; j++) {
        for (int i = 0; i < 26; i++) {
            int tile0 = tiledMap.getTile(i + camera_x, j + camera_y, 0);
            int tile1 = tiledMap.getTile(i + camera_x, j + camera_y, 1);

            if (tile0 < (int)tileset->spriteset.size()) {
                tileset->spriteset[tile0]->draw(screen,
                        i * 32 - offset_x, j * 32 - offset_y);
            }
            if (tile1 > 0 && tile1 < (int)tileset->spriteset.size()) {
                tileset->spriteset[tile1]->draw(screen,
                        i * 32 - offset_x, j * 32 - offset_y);
            }
            
        }
    }
        
    // Draw nodes
    std::list<Being*>::iterator beingIterator = beings.begin();
    while (beingIterator != beings.end()) {
        Being *being = (*beingIterator);

        unsigned short x = being->x;
        unsigned short y = being->y;
        unsigned char dir = being->direction / 2;
        int sx = x - camera_x;
        int sy = y - camera_y;
    
#ifdef DEBUG
        //rect(screen, sx*32, sy*32, sx*32+32, sy*32+32, makecol(0,0,255));
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
                pf += 4 * being->weapon;
            }

            playerset->spriteset[4 * pf + dir]->draw(screen,
                    being->text_x - 64, being->text_y - 80);

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
                    being->frame = 0;
                    being->action = STAND;
                    if (being->id == player_node->id) {
                        walk_status = 0;
                    }
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
                monsterset->spriteset[sprnum + 8 * MONSTER_DEAD]->draw(screen,
                        being->text_x + 30, being->text_y + 40);
            }
            else {
                monsterset->spriteset[sprnum + 8 * mf]->draw(screen,
                        being->text_x + 30, being->text_y + 40);
            }

            if (being->action != STAND) {
                being->frame =
                    (get_elapsed_time(being->walk_time) * 4) / (being->speed);

                if (being->frame >= 4) {
                    if (being->action != MONSTER_DEAD && being->hasPath()) {
                        being->nextStep();
                    }
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

    // Draw tiles above nodes
    for (int j = 0; j < 20; j++) {
        for (int i = 0; i < 26; i++) {
            int tile = tiledMap.getTile(i + camera_x, j + camera_y, 2);

            if (tile > 0 && tile < (int)tileset->spriteset.size()) {
                tileset->spriteset[tile]->draw(
                        screen, i * 32 - offset_x, j * 32 - offset_y);
            }
#ifdef DEBUG            
            //rect(screen, i * 32 - offset_x, j * 32 - offset_y,
            //        i * 32 - offset_x + 32, j * 32 - offset_y + 32,
            //        makecol(0,0,0));
#endif
        }
    }

#ifdef __DEBUG
    // Draw a debug path
    PATH_NODE *debugPath = tiledMap.findPath(
            player_node->x, player_node->y,
            mouseX / 32 + camera_x, mouseY / 32 + camera_y);

    while (debugPath)
    {
        SDL_Rect destRect;
        destRect.x = (debugPath->x - camera_x) * 32 - offset_x + 12;
        destRect.y = (debugPath->y - camera_y) * 32 - offset_y + 12;
        destRect.w = 8;
        destRect.h = 8;
        SDL_FillRect(screen, &destRect, SDL_MapRGB(screen->format, 255, 0, 0));

        Tile *tile = tiledMap.getTile(debugPath->x, debugPath->y);

        std::stringstream cost;
        cost << tile->Gcost;
        guiGraphics->_beginDraw();
        guiGraphics->drawText(cost.str(), destRect.x + 4, destRect.y + 12,
                gcn::Graphics::CENTER);
        guiGraphics->_endDraw();

        // Move to the next node
        PATH_NODE *temp = debugPath->next;
        delete debugPath;
        debugPath = temp;
    }
#endif
    
    // Draw player speech
    beingIterator = beings.begin();
    while (beingIterator != beings.end()) {
        Being *being = (*beingIterator);

        if (being->speech != NULL) {
            guiGraphics->_beginDraw();
            //if (being->speech_color == makecol(255, 255, 255)) {
            //    guiGraphics->drawText(being->speech,
            //            being->text_x + 16, being->text_y - 60,
            //            gcn::Graphics::CENTER);
            //}
            //else {
                guiGraphics->drawText(being->speech,
                        being->text_x + 60, being->text_y - 60,
                        gcn::Graphics::CENTER);
            //}
            guiGraphics->_endDraw();

            being->speech_time--;
            if (being->speech_time == 0) {
                free(being->speech);
                being->speech = NULL;
            }
        }

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
    debugInfo->setCaption(debugStream.str());
    debugInfo->adjustSize();
}
