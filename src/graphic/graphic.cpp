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

BITMAP *buffer, *chat_background;

char itemCurrenyQ[10] = "0";
int map_x, map_y, camera_x, camera_y;
DIALOG_PLAYER *skill_player, *skill_list_player;
char npc_text[1000] = "";
char statsString2[255] = "n/a";
char skill_points[10] = "";
Chat chatlog("./docs/chatlog.txt", 20);
bool show_skill_dialog = false;
bool show_skill_list_dialog = false;
char npc_button[10] = "Close";

gcn::TextField *chatInput;

StatusWindow *statusWindow;
BuyDialog *buyDialog;
SellDialog *sellDialog;
BuySellDialog *buySellDialog;
InventoryWindow *inventoryWindow;
NpcListDialog *npcListDialog;
NpcTextDialog *npcTextDialog;
SkillDialog *skillDialog;

void ChatListener::action(const std::string& eventId)
{
    if (eventId == "chatinput") {
        std::string message = chatInput->getText();

        if (message.length() > 0) {
            chatlog.chat_send(char_info[0].name, message.c_str());
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

/*
DIALOG skill_list_dialog[] = {
   // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)                    (d2)  (dp)              (dp2) (dp3) 
   { tmw_dialog_proc,     300,  200,  260,  200,  0,    0,    0,    0,       0,                0,    (char *)"Stats",     NULL, NULL  },
   { tmw_button_proc,     450,  376,  50,   20,   255,  0,    'u',  D_EXIT,  0,                0,    (char *)"&Up",        NULL, NULL  },
   { tmw_button_proc,     508,  376,  50,   20,   255,  0,    'c',  D_EXIT,  0,			           0,    (char *)"&Close",     NULL, NULL  },
   { tmw_list_proc,       304,  224,  252,  100,  0,    0,    0,    0,       0,                0,    (char *)skill_list,   NULL, NULL  },
   { tmw_text_proc,       304,  326,  40,   20,   0,    0,    0,    0,       0,                0,    (char *)skill_points, NULL, NULL  },
   { NULL,                0,    0,    0,    0,    0,    0,    0,    0,       0,                0,    NULL,                 NULL, NULL  }
};
*/

char hairtable[14][4][2] = {
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
    { { 0, 4}, {-1, 6}, {-1, 6}, {0, 6} } // SIT
};

int get_x_offset(NODE *node) {
    int offset = 0;
    char direction = get_direction(node->coordinates);
    if (node->action == WALK) {
        if (direction != NORTH && direction != SOUTH) {
            offset = node->frame + 1;
            if (offset == 5)offset = 0;
            offset *= 8;
            if (direction == WEST || direction == NW || direction == SW) {
                offset = -offset;
                offset += 32;
            } else offset -= 32;
        }
    }
    return offset;
}

int get_y_offset(NODE *node) {
    int offset = 0;
    char direction = get_direction(node->coordinates);
    if (node->action == WALK) {
        if (direction != EAST && direction != WEST) {
            offset = node->frame + 1;
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


GraphicEngine::GraphicEngine() {
    clear_bitmap(screen);
    
    // Initializes GUI
    chat_background = create_bitmap(592, 100);
    clear_to_color(chat_background, makecol(0, 0, 0));
    chatInput = new TextField();
    chatInput->setPosition(chatInput->getBorderSize(),
        SCREEN_H - chatInput->getHeight() - chatInput->getBorderSize() -1);
    chatInput->setWidth(592 - 2 * chatInput->getBorderSize());

    ChatListener *chatListener = new ChatListener();
    chatInput->setEventId("chatinput");
    chatInput->addActionListener(chatListener);

    guiTop->add(chatInput);

    // Create dialogs

    statusWindow = new StatusWindow(guiTop);
    statusWindow->setPosition(SCREEN_W - statusWindow->getWidth() - 10, 10);

    buyDialog = new BuyDialog(guiTop);
    buyDialog->setVisible(false);

    sellDialog = new SellDialog(guiTop);
    sellDialog->setVisible(false);

    buySellDialog = new BuySellDialog(guiTop, new BuySellListener());
    buySellDialog->setVisible(false);

    inventoryWindow = new InventoryWindow(guiTop);
    inventoryWindow->setVisible(false);
    inventoryWindow->setPosition(100, 100);

    npcTextDialog = new NpcTextDialog(guiTop);
    npcTextDialog->setVisible(false);

    npcListDialog = new NpcListDialog(guiTop);
    npcListDialog->setVisible(false);

    skillDialog = new SkillDialog(guiTop);
    skillDialog->setVisible(false);

    //skill_player = init_dialog(skill_dialog, -1);
    //skill_list_player = init_dialog(skill_list_dialog, -1);

    // Give focus to the chat input
    chatInput->requestFocus();

    // SDL probably doesn't need this buffer, it'll buffer for us.
    buffer = create_bitmap(SCREEN_W, SCREEN_H);
    if (!buffer) {
        error("Not enough memory to create buffer");
    }

    // Initialize the gui bitmap to the page that will be drawn first
    gui_bitmap = this->buffer;
    
    emotionset = new Spriteset("./data/graphic/emotionset.dat");
    npcset = new Spriteset("./data/graphic/npcset.dat");
    tileset = new Spriteset("./data/graphic/tileset.dat");

    BITMAP *monsterbitmap = load_bitmap("data/graphic/monsterset.bmp", NULL);
    if (!monsterbitmap) error("Unable to load monsterset.bmp");
    monsterset = new Spriteset(monsterbitmap, 60, 60, 30, 40);
}

GraphicEngine::~GraphicEngine() {
    delete statusWindow;
    delete buyDialog;
    delete sellDialog;
    delete buySellDialog;
    delete npcListDialog;
    delete npcTextDialog;
    delete skillDialog;
    
    //delete tileset;

    //shutdown_dialog(skill_player);
}

void GraphicEngine::refresh() {
    map_x = (get_x(player_node->coordinates) - 13) * 32 +
        get_x_offset(player_node);
    map_y = (get_y(player_node->coordinates) - 9) * 32 +
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
            unsigned short tile0 = get_tile(i + camera_x, j + camera_y, 0);
            unsigned short tile1 = get_tile(i + camera_x, j + camera_y, 1);

            if (tile0 < 600) {
                tileset->spriteset[tile0]->draw(buffer,
                i * 32 - offset_x, j * 32 - offset_y);
            }
            if (tile1 > 0) { //&& tile1 < 600
                tileset->spriteset[tile1]->draw(buffer,
                i * 32 - offset_x, j * 32 - offset_y);
            }
            
        }
    }
        
    // Draw nodes
    std::list<NODE*>::iterator beingIterator = beings.begin();
    while (beingIterator != beings.end()) {
        NODE *node = (*beingIterator);

        unsigned short x = get_x(node->coordinates);
        unsigned short y = get_y(node->coordinates);
        unsigned char dir = get_direction(node->coordinates) / 2;
        int sx = x - camera_x;
        int sy = y - camera_y;
    
#ifdef DEBUG
        textprintf_ex(buffer, font, sx*32, sy*32+40, makecol(255, 255, 255), -1, "%i,%i | %i", x, y, node->frame);
        rect(buffer, sx*32, sy*32, sx*32+32, sy*32+32, makecol(0,0,255));
#endif
        
        if ((node->job >= 100) && (node->job <= 110)) { // Draw a NPC
            npcset->spriteset[4 * (node->job - 100) + dir]->draw(buffer,
                    sx * 32 - 8 - offset_x,
                    sy * 32 - 52 - offset_y);
        }
        else if (node->job < 10) { // Draw a player
            node->text_x = sx * 32 + get_x_offset(node) - offset_x;
            node->text_y = sy * 32 + get_y_offset(node) - offset_y;
            int hf = node->hair_color - 1 + 10 * (dir + 4 *
                    (node->hair_style - 1));
            
            if (node->action == SIT) node->frame = 0;
            if (node->action == ATTACK) {
                int pf = node->frame + node->action + 4 * node->weapon;

                playerset->spriteset[4 * pf + dir]->draw(buffer,
                        node->text_x - 64, node->text_y - 80);
                hairset->spriteset[hf]->draw(buffer,
                        node->text_x - 2 + 2 * hairtable[pf][dir][0],
                        node->text_y - 50 + 2 * hairtable[pf][dir][1]);
            }
            else {
                int pf = node->frame + node->action;

                playerset->spriteset[4 * pf + dir]->draw(buffer,
                        node->text_x - 64, node->text_y - 80);
                hairset->spriteset[hf]->draw(buffer,
                        node->text_x - 2 + 2 * hairtable[pf][dir][0],
                        node->text_y - 50 + 2 * hairtable[pf][dir][1]);
            }
            if (node->emotion != 0) {
                emotionset->spriteset[node->emotion - 1]->draw(buffer,
                        sx * 32 - 5 + get_x_offset(node) - offset_x,
                        sy * 32 - 45 + get_y_offset(node) - offset_y);
                node->emotion_time--;
                if (node->emotion_time == 0) {
                    node->emotion = 0;
                }
            }
            if (node->action != STAND && node->action != SIT) {
                node->frame =
                    (get_elapsed_time(node->tick_time) * 4) / (node->speed);

                if (node->frame >= 4) {
                    node->frame = 0;
                    node->action = STAND;
                    if (node->id == player_node->id) {
                        walk_status = 0;
                    }
                }
            }
        }
        else if (node->job == 45) { // Draw a warp
        } else { // Draw a monster
            if (node->frame >= 4)
                node->frame = 3;

            node->text_x = sx * 32 - 42 + get_x_offset(node) - offset_x;
            node->text_y = sy * 32 - 65 + get_y_offset(node) - offset_y;

            int sprnum = dir + 4 * (node->job - 1002);
            int mf = node->frame + node->action;

            if (node->action == MONSTER_DEAD) {
                monsterset->spriteset[sprnum + 8 * MONSTER_DEAD]->draw(
                        buffer, node->text_x, node->text_y);
            }
            else {
                monsterset->spriteset[sprnum + 8 * mf]->draw(buffer,
                        node->text_x, node->text_y);
            }

            if (node->action != STAND) {
                node->frame =
                    (get_elapsed_time(node->tick_time) * 4) / (node->speed);

                if (node->frame >= 4) {
                    if (node->action != MONSTER_DEAD && node->path) {
                        if (node->path->next) {
                            int old_x, old_y, new_x, new_y;
                            old_x = node->path->x;
                            old_y = node->path->y;
                            node->path = node->path->next;
                            new_x = node->path->x;
                            new_y = node->path->y;
                            direction = 0;

                            if (new_x > old_x) {
                                if (new_y > old_y)      direction = SE;
                                else if (new_y < old_y) direction = NE;
                                else                    direction = EAST;
                            }
                            else if (new_x < old_x) {
                                if (new_y > old_y)      direction = SW;
                                else if (new_y < old_y) direction = NW;
                                else                    direction = WEST;
                            }
                            else {
                                if (new_y > old_y)      direction = SOUTH;
                                else if (new_y < old_y) direction = NORTH;
                            }

                            set_coordinates(node->coordinates,
                                    node->path->x, node->path->y, direction);
                        } else {
                            node->action = STAND;
                        }
                        if (node->action != MONSTER_DEAD) {
                            node->frame = 0;
                        }
                        node->tick_time = tick_time;
                        //node->frame = 0;
                    }
                }
            }
        }
        
        if (node->action == MONSTER_DEAD && node->frame >= 20) {
            delete node;
            beingIterator = beings.erase(beingIterator);
        }
        else {
            beingIterator++;
        }

        // nodes are ordered so if the next node y is > then the
        // last drawed for fringe layer, draw the missing lines
    }

    // Draw tiles above nodes
    for (int j = 0; j < 20; j++) {
        for (int i = 0; i < 26; i++) {
            unsigned short tile = get_tile(i + camera_x, j + camera_y, 2);

            if (tile > 0 && tile < 600) {
                tileset->spriteset[tile]->draw(
                        buffer, i * 32 - offset_x, j * 32 - offset_y);
            }
#ifdef DEBUG            
            rect(buffer, i * 32 - offset_x, j * 32 - offset_y,
                    i * 32 - offset_x + 32, j * 32 - offset_y + 32,
                    makecol(0,0,0));
#endif
        }
    }
    
    // Draw player speech
    beingIterator = beings.begin();
    while (beingIterator != beings.end()) {
        NODE *node = (*beingIterator);

        if (node->speech != NULL) {
            if (node->speech_color == makecol(255, 255, 255)) {
                textprintf_centre_ex(buffer, font,
                        node->text_x,
                        node->text_y - 60,
                        node->speech_color, -1,
                        "%s", node->speech);
            }
            else {
                textprintf_centre_ex(buffer, font,
                        node->text_x + 60,
                        node->text_y,
                        node->speech_color, -1,
                        "%s", node->speech);
            }

            node->speech_time--;
            if (node->speech_time == 0) {
                free(node->speech);
                node->speech = NULL;
            }
        }

        beingIterator++;
    }

    set_trans_blender(0, 0, 0, 110);
    draw_trans_sprite(buffer, chat_background, 0, SCREEN_H - 125);

    chatlog.chat_draw(buffer, 8, font);
    
    /*
    if (show_skill_dialog) {
        update_skill_dialog();
        if (gui_update(skill_player) == 0) {
            show_skill_dialog = false;
        }
    }
    */

    /*
    if (show_skill_list_dialog) {
        if (gui_update(skill_list_player) == 0) {
            int ret = shutdown_dialog(skill_list_player);
            if (ret == 1) {
                if (char_info->skill_point > 0) {
                    WFIFOW(0) = net_w_value(0x0112);
                    WFIFOW(2) = net_w_value(
                            get_skill_id(skill_list_dialog[3].d1));
                    WFIFOSET(4);
                }
            } else if(ret == 2) {
                show_skill_list_dialog = false;
            }
            skill_list_player = init_dialog(skill_list_dialog, -1);
        }
    }
    */

    // Update character status display
    statusWindow->update();

    // Update GUI
    guiGraphics->setTarget(buffer);
    gui_update(NULL);

    textprintf_ex(buffer, font, 0, 0, makecol(255, 255, 255), -1,
            "[%i fps] %i,%i", fps,
            mouse_x / 32 + camera_x, mouse_y / 32 + camera_y);

    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}
