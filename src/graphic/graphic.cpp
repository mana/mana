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
 *  By ElvenProgrammer aka Eugenio Favalli (umperio@users.sourceforge.net)
 */

#include "graphic.h"
#include "2xsai.h"
#include "../gui/gui.h"
#include "../gui/stats.h"
#include "../gui/textfield.h"

#define TILESET_W 480
#define TILESET_H 320

#ifdef WIN32
#pragma warning (disable:4312)
#endif

BITMAP *vbuffer, *vpage[2], *vtileset, *temp, *vnpcset, *vplayerset, *vmonsterset;
int page_num = 0;
bool page_flipping;

BITMAP *buffer, *double_buffer, *chat_background;
DATAFILE *tileset;

char itemCurrenyQ[10] = "0";
//char page_num;
int map_x, map_y, camera_x, camera_y;
DIALOG_PLAYER *npc_player, *skill_player, *buy_sell_player, *buy_player, *sell_player, *skill_list_player, *npc_list_player;
char npc_text[1000] = "";
char statsString2[255] = "n/a";
char skill_points[10] = "";
TmwInventory inventory;
Chat chatlog("./docs/chatlog.txt", 20);
int show_npc_dialog = 0;
bool show_skill_dialog = false;
bool show_skill_list_dialog = false;
char npc_button[10] = "Close";

gcn::TextField *chatInput;
StatsDialog *statsDialog;

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


Spriteset *new_tileset, *new_playerset, *new_npcset, *new_emotionset;
Spriteset *new_monsterset;

DIALOG npc_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,     300,  200,  260,  150,  0,    0,    0,    0,       0,                0,    (char *)"NPC",             NULL, NULL  },
   { tmw_button_proc,     508,  326,  50,   20,   255,  0,    'c',  D_EXIT,  0,                0,    (char *)npc_button,         NULL, NULL  },
   { tmw_textbox_proc,    304,  224,  252,  100,  0,    0,    0,    0,       0,                0,    npc_text,         NULL, NULL  },
   { NULL,                0,    0,    0,    0,    0,    0,    0,    0,       0,                0,    NULL,             NULL, NULL  }
};


DIALOG buy_sell_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,     350,  200,  100,  105,  0,    0,    0,    0,       0,                0,    (char *)"Shop",   NULL, NULL  },
   { tmw_button_proc,     360,  225,  80,   20,   255,  0,    0,    D_EXIT,  0,                0,    (char *)"&Buy",   NULL, NULL  },
	 { tmw_button_proc,     360,  250,  80,   20,   255,  0,    0,    D_EXIT,  0,                0,    (char *)"&Sell",  NULL, NULL  },
	 { tmw_button_proc,     360,  275,  80,   20,   255,  0,    0,    D_EXIT,  0,                0,    (char *)"&Cancel",NULL, NULL  },
   { NULL,                0,    0,    0,    0,    0,    0,    0,    0,       0,                0,    NULL,             NULL, NULL  }
};

DIALOG buy_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,     300,  200,  260,  200,  0,    0,    0,    0,       0,                0,    (char *)"Buy",     NULL, NULL  },
   { tmw_button_proc,     450,  376,  50,   20,   255,  0,    'o',  D_EXIT,  0,                0,    (char *)"&Ok",     NULL, NULL  },
	 { tmw_button_proc,     508,  376,  50,   20,   255,  0,    'c',  D_EXIT,  0,                0,    (char *)"&Cancel", NULL, NULL  },
   { tmw_list_proc,       304,  224,  252,  100,  0,    0,    0,    0,       0,                0,    (char *)shop_list, NULL, NULL  },
	 { tmw_text_proc,       304,  326+25,  50,  20,  0,    0,    0,    0,       0,                0,    NULL,              NULL, NULL  },
   { tmw_slider_proc,     304,  326,  200,   20,   255,  0,    0,  0,		10,				   0,	 NULL,   (void *)changeQ, NULL  },
   { tmw_text_proc,       514,  326,  40,  20,  0,    0,    0,    0,       0,                0,    (char *)itemCurrenyQ, NULL, NULL  },
   { NULL,                0,    0,    0,    0,    0,    0,    0,    0,       0,                0,    NULL,              NULL, NULL  }
};

DIALOG sell_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,     300,  200,  260,  200,  0,    0,    0,    0,       0,                0,    (char *)"Sell",    NULL, NULL  },
   { tmw_button_proc,     450,  376,  50,   20,   255,  0,    'o',  D_EXIT,  0,             0,    (char *)"&Ok",     NULL, NULL  },
   { tmw_button_proc,     508,  376,  50,   20,   255,  0,    'c',  D_EXIT,  0,			   0,    (char *)"&Cancel", NULL, NULL  },
   { tmw_slider_proc,     304,  326,  200,   20,   255,  0,    0,  0,		10,				   0,	 NULL,   (void *)changeQ, NULL  },
   { tmw_list_proc,       304,  224,  252,  100,  0,    0,    0,    0,       0,                0,    (char *)shop_list, NULL, NULL  },
   { tmw_text_proc,       514,  326,  40,  20,  0,    0,    0,    0,       0,                0,    (char *)itemCurrenyQ, NULL, NULL  },
   { NULL,                0,    0,    0,    0,    0,    0,    0,    0,       0,                0,    NULL,              NULL, NULL  }
};

DIALOG skill_list_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,     300,  200,  260,  200,  0,    0,    0,    0,       0,                0,    (char *)"Skills",     NULL, NULL  },
   { tmw_button_proc,     450,  376,  50,   20,   255,  0,    'u',  D_EXIT,  0,                0,    (char *)"&Up",        NULL, NULL  },
   { tmw_button_proc,     508,  376,  50,   20,   255,  0,    'c',  D_EXIT,  0,			           0,    (char *)"&Close",     NULL, NULL  },
   { tmw_list_proc,       304,  224,  252,  100,  0,    0,    0,    0,       0,                0,    (char *)skill_list,   NULL, NULL  },
   { tmw_text_proc,       304,  326,  40,   20,   0,    0,    0,    0,       0,                0,    (char *)skill_points, NULL, NULL  },
   { NULL,                0,    0,    0,    0,    0,    0,    0,    0,       0,                0,    NULL,                 NULL, NULL  }
};

DIALOG npc_list_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,     300,  200,  260,  200,  0,    0,    0,    0,       0,                0,    (char *)"NPC",    NULL, NULL  },
   { tmw_button_proc,     450,  376,  50,   20,   255,  0,    'o',  D_EXIT,  0,             0,    (char *)"&Ok",     NULL, NULL  },
   { tmw_button_proc,     508,  376,  50,   20,   255,  0,    'c',  D_EXIT,  0,			   0,    (char *)"&Cancel", NULL, NULL  },
   { tmw_list_proc,       304,  224,  252,  100,  0,    0,    0,    0,       0,                0,    (char *)item_list, NULL, NULL  },
   { NULL,                0,    0,    0,    0,    0,    0,    0,    0,       0,                0,    NULL,              NULL, NULL  }
};

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

void set_npc_dialog(int show) {
    show_npc_dialog = show;
}

int get_x_offset(NODE *node) {
    int offset = 0;
    char direction = get_direction(node->coordinates);
    if (node->action == WALK) {
        if (direction!=NORTH && direction!=SOUTH) {
            offset = node->frame + 1;
            if (offset==5)offset = 0;
            offset *= 4;
            if (direction == WEST || direction == NW || direction == SW) {
                offset = -offset;
                offset += 16;
            } else offset -= 16;
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
            offset *= 4;
            if (direction == NORTH || direction == NW || direction == NE) {
                offset = -offset;
                offset += 16;
            }
            else {
                offset -= 16;
            }
        }
    }
    return offset;
}

void init_graphic() {
    clear_bitmap(screen);
    chat_background = create_bitmap(592, 100);
    clear_to_color(chat_background, makecol(0, 0, 0));

    // Initialize gui

    // Create chat input field
    chatInput = new TextField();
    chatInput->setPosition(
            chatInput->getBorderSize(),
            SCREEN_H - chatInput->getHeight() - chatInput->getBorderSize() -1);
    chatInput->setWidth(592 - 2 * chatInput->getBorderSize());

    ChatListener *chatListener = new ChatListener();
    chatInput->setEventId("chatinput");
    chatInput->addActionListener(chatListener);

    guiTop->add(chatInput);

    chatInput->requestFocus();

    // Create stats dialog
    statsDialog = new StatsDialog(guiTop);
    statsDialog->setPosition(SCREEN_W - statsDialog->getWidth() - 10, 10);


    npc_player = init_dialog(npc_dialog, -1);
    position_dialog(npc_dialog, 300, 200);
    skill_player = init_dialog(skill_dialog, -1);
    buy_sell_player = init_dialog(buy_sell_dialog, -1);
    buy_player = init_dialog(buy_dialog, -1);
    sell_player = init_dialog(sell_dialog, -1);
    skill_list_player = init_dialog(skill_list_dialog, -1);
    npc_list_player = init_dialog(npc_list_dialog, -1);
    inventory.create(100, 100);

    vpage[0] = NULL;
    vpage[1] = NULL;
    page_flipping = false;

    // Try to set up two video bitmaps for high performance drawing when
    // simple stretching is used and hardware vram blit is possible.
    if ((gfx_capabilities & GFX_HW_VRAM_BLIT) && stretch_mode == 0)
    {
        vpage[0] = create_video_bitmap(SCREEN_W, SCREEN_H);
        vpage[1] = create_video_bitmap(SCREEN_W, SCREEN_H);

        if (!vpage[0] || !vpage[1])
        {
            warning("Not enough video memory for page flipping");

            if (vpage[0]) {
                destroy_bitmap(vpage[0]);
                vpage[0] = NULL;
            }
            if (vpage[1]) {
                destroy_bitmap(vpage[1]);
                vpage[1] = NULL;
            }
        }
        else {
            page_flipping = true;
        }
    }

    // No videobitmap pages were created, so create a normal bitmap that'll
    // be used as buffer.
    if (!page_flipping) {
        vpage[0] = create_bitmap(SCREEN_W, SCREEN_H);
        vpage[1] = vpage[0];
        if (!vpage[0]) error("Not enough memory for screen buffer");
    }

    // Create a buffer to draw the low resolution map on
    vbuffer = create_bitmap(SCREEN_W / 2, SCREEN_H / 2);
    if (!vbuffer) error("Not enough memory for map buffer");

    // Initialize the gui bitmap to the page that will be drawn first
    gui_bitmap = vpage[page_num];

    new_emotionset = new Spriteset("./data/graphic/emotionset.dat");
    new_tileset = new Spriteset("./data/graphic/desert.dat");
    new_npcset = new Spriteset("./data/graphic/npcset.dat");
    new_playerset = new Spriteset("./data/graphic/playerset.dat");
    new_monsterset = new Spriteset("./data/graphic/monsterset.dat");
}

void do_graphic(void) {
    map_x = (get_x(player_node->coordinates) - 13) * 16 +
        get_x_offset(player_node);
    map_y = (get_y(player_node->coordinates) - 9) * 16 +
        get_y_offset(player_node);

    camera_x = map_x >> 4;
    camera_y = map_y >> 4;

    int offset_x = map_x & 15;
    int offset_y = map_y & 15;

    sort();

    frame++;
    acquire_bitmap(vbuffer);

    new_tileset->spriteset[0]->draw(vbuffer, 0, 0);

    // Draw tiles below nodes
    for (int j = 0; j < 20; j++) {
        for (int i = 0; i < 26; i++) {
            unsigned short tile0 = get_tile(i + camera_x, j + camera_y, 0);
            unsigned short tile1 = get_tile(i + camera_x, j + camera_y, 1);

            if (tile0 < 600) {
                new_tileset->spriteset[tile0]->draw(
                        vbuffer, i * 16 - offset_x, j * 16 - offset_y);
            }
            if (tile1 > 0) { //&& tile1 < 600
                new_tileset->spriteset[tile1]->draw(
                        vbuffer, i * 16 - offset_x, j * 16 - offset_y);
            }
        }
    }

    // Draw nodes
    NODE *node = get_head();
    while (node) {
        unsigned short x = get_x(node->coordinates);
        unsigned short y = get_y(node->coordinates);
        unsigned char dir = get_direction(node->coordinates);
        int sx = x - camera_x;
        int sy = y - camera_y;
        
        if ((node->job >= 100) && (node->job <= 110)) { // Draw a NPC
            new_npcset->spriteset[4 * (node->job - 100) + dir / 2]->draw(vbuffer, sx * 16 - 4 - offset_x, sy * 16 - 24 - offset_y);
        }
        else if (node->job < 10) { // Draw a player
            node->text_x = sx * 16 - 34 + get_x_offset(node) - offset_x;
            node->text_y = sy * 16 - 36 + get_y_offset(node) - offset_y;
            if (node->action == SIT) node->frame = 0;
            if (node->action == ATTACK) {
                new_playerset->spriteset[(dir / 2) + 4 * (node->frame + node->action + 4 * node->weapon)]->draw(vbuffer, node->text_x, node->text_y);
                draw_rle_sprite(vbuffer, (RLE_SPRITE*)weaponset[16 * node->weapon + 4 * node->frame + dir / 2].dat, node->text_x, node->text_y);
                masked_blit(hairset, vbuffer, 20 * (node->hair_color - 1), 20 * (dir / 2 + 4 * (node->hair_style - 1)), node->text_x + 31 + hairtable[node->action + node->frame + 4 * node->weapon][dir / 2][0], node->text_y + 15 + hairtable[node->action + node->frame + 4 * node->weapon][dir / 2][1], 20, 20);
            }
            else {
                new_playerset->spriteset[4 * (node->frame + node->action) + dir / 2]->draw(vbuffer, node->text_x, node->text_y);
                masked_blit(hairset, vbuffer, 20 * (node->hair_color - 1), 20 * (dir / 2 + 4 * (node->hair_style - 1)), node->text_x + 31 + hairtable[node->action + node->frame][dir / 2][0], node->text_y + 15 + hairtable[node->action + node->frame][dir / 2][1], 20, 20);
            }
            if (node->emotion != 0) {
                new_emotionset->spriteset[node->emotion - 1]->draw(vbuffer, sx * 16 - 5 + get_x_offset(node) - offset_x, sy * 16 - 45 + get_y_offset(node) - offset_y);
                node->emotion_time--;
                if (node->emotion_time == 0) {
                    node->emotion = 0;
                }
            }
            if (node->action != STAND && node->action != SIT) {
                node->frame = (get_elapsed_time(node->tick_time) * 4) / (node->speed);
                if (node->frame >= 4) {
                    node->frame = 0;
                    node->action = STAND;
                    if (node->id == player_node->id) {
                        walk_status = 0;
                    }
                }
            }

            textprintf_centre_ex(vbuffer, font,
                    node->text_x + 45,
                    node->text_y + 55,
                    node->speech_color, -1,
                    "(%d,%d)", x, y);
        }
        else if (node->job == 45) { // Draw a warp
        } else { // Draw a monster
            if (node->frame >= 4)
                node->frame = 3;

            node->text_x = sx * 16 - 22 + get_x_offset(node) - offset_x;
            node->text_y = sy * 16 - 25 + get_y_offset(node) - offset_y;

            //int r_x = node->text_x-get_x_offset(node);
            //int r_y = node->text_y-get_y_offset(node);

            if (node->action == MONSTER_DEAD) {
                new_monsterset->spriteset[(dir / 2) + 4 * (node->job - 1002) + 8 * MONSTER_DEAD]->draw(vbuffer, node->text_x, node->text_y);
            }
            else {
                new_monsterset->spriteset[(dir / 2) + 4 * (node->job - 1002) + 8 * (node->frame + node->action)]->draw(vbuffer, node->text_x, node->text_y);
            }

            if (node->action != STAND) {
                node->frame = (get_elapsed_time(node->tick_time) * 4) / (node->speed);
                if (node->frame >= 4) {
                    if (node->action != MONSTER_DEAD && node->path) {
                        if (node->path->next) {
                            PATH_NODE *old = node->path;
                            node->path = node->path->next;
                            direction = 0;
                            //if(node->path->next) {
                            if (node->path->x>old->x && node->path->y>old->y)direction = SE;
                            else if (node->path->x<old->x && node->path->y>old->y)direction = SW;
                            else if (node->path->x>old->x && node->path->y<old->y)direction = NE;
                            else if (node->path->x<old->x && node->path->y<old->y)direction = NW;
                            else if (node->path->x>old->x)direction = EAST;
                            else if (node->path->x<old->x)direction = WEST;
                            else if (node->path->y>old->y)direction = SOUTH;
                            else if (node->path->y<old->y)direction = NORTH;
                            //}

                            set_coordinates(node->coordinates, node->path->x, node->path->y, direction);

                            //node->tick_time = tick_time;
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
            NODE *temp = node;
            node = node->next;
            remove_node(temp->id);
        }
        else {
            node = node->next;
        }

        // nodes are ordered so if the next node y is > then the
        // last drawed for fringe layer, draw the missing lines
    }

    // Draw tiles above nodes
    for (int j = 0; j < 20; j++) {
        for (int i = 0; i < 26; i++) {
            unsigned short tile = get_tile(i + camera_x, j + camera_y, 2);

            if (tile > 0 && tile < 600) {
                new_tileset->spriteset[tile]->draw(
                        vbuffer, i * 16 - offset_x, j * 16 - offset_y);
            }
        }
    }

    release_bitmap(vbuffer);
    acquire_bitmap(vpage[page_num]);

    if (stretch_mode == 0) {
        stretch_blit(vbuffer, vpage[page_num], 0, 0, 400, 300, 0, 0, 800, 600);
    }
    else if (stretch_mode == 1) {
        Super2xSaI(vbuffer, vpage[page_num], 0, 0, 0, 0, 400, 300);
    }
    else if (stretch_mode == 2) {
        SuperEagle(vbuffer, vpage[page_num], 0, 0, 0, 0, 400, 300);
    }

    textprintf_ex(vpage[page_num], font, 0, 0, makecol(255, 255, 255), -1,
            "[%i fps]", fps);

    // Draw player speech
    node = get_head();
    while (node) {
        if (node->speech != NULL) {
            if (node->speech_color == makecol(255, 255, 255)) {
                textprintf_centre_ex(vpage[page_num], font,
                        node->text_x * 2 + 90,
                        node->text_y * 2,
                        node->speech_color, -1,
                        "%s", node->speech);
            }
            else {
                textprintf_centre_ex(vpage[page_num], font,
                        node->text_x * 2 + 60,
                        node->text_y * 2,
                        node->speech_color, -1,
                        "%s", node->speech);
            }

            node->speech_time--;
            if (node->speech_time == 0) {
                free(node->speech);
                node->speech = NULL;
            }
        }
        node = node->next;
    }

    // Update character status display
    statsDialog->update();

    // Update GUI
    guiGraphics->setTarget(vpage[page_num]);
    gui_update(NULL);

    inventory.draw(vpage[page_num]);

    set_trans_blender(0, 0, 0, 110);
    draw_trans_sprite(vpage[page_num], chat_background, 0, SCREEN_H - 125);

    chatlog.chat_draw(vpage[page_num], 8, font);

    switch (show_npc_dialog) {
        case 1:
            dialog_message(npc_dialog, MSG_DRAW, 0, 0);
            if (!(show_npc_dialog = gui_update(npc_player))) {
                strcpy(npc_text, "");
                WFIFOW(0) = net_w_value(0x00b9);
                //alert("","","","","",0,0);
                WFIFOL(2) = net_l_value(current_npc);
                WFIFOSET(6);
            }
            break;
        case 2:
            dialog_message(buy_sell_dialog, MSG_DRAW, 0, 0);
            if (!gui_update(buy_sell_player)) {
                show_npc_dialog = shutdown_dialog(buy_sell_player);
                if (show_npc_dialog == 1 || show_npc_dialog == 2) {
                    WFIFOW(0) = net_w_value(0x00c5);
                    WFIFOL(2) = net_l_value(current_npc);
                    WFIFOB(6) = net_b_value(show_npc_dialog-1);
                    WFIFOSET(7);
                }
                show_npc_dialog = 0;
                buy_sell_player = init_dialog(buy_sell_dialog, -1);
            }
            break;
        case 3:
            char money[20];
            sprintf(money, "%i gp", char_info->gp);
            buy_dialog[4].dp = &money;
            buy_dialog[5].d1 = (int)(char_info->gp/get_item_price(buy_dialog[3].d1));
            if (buy_dialog[5].d2>buy_dialog[5].d1)
                dialog_message(buy_dialog, MSG_DRAW, 0, 0);
            if (!gui_update(buy_player)) {
                show_npc_dialog = shutdown_dialog(buy_player);
                buy_dialog[5].d1 = 0;
                if(show_npc_dialog==1) {
                    WFIFOW(0) = net_w_value(0x00c8);
                    WFIFOW(2) = net_w_value(8);
                    WFIFOW(4) = net_w_value(buy_dialog[5].d2);
                    WFIFOW(6) = net_w_value(get_item_id(buy_dialog[3].d1));
                    WFIFOSET(8);
                }
                show_npc_dialog = 0;
                buy_player = init_dialog(buy_dialog, -1);
                close_shop();
            }
            break;
        case 4:
            //alert("","","","","",0,0);
            //char ds[20];
            sell_dialog[3].d1 = get_item_quantity(sell_dialog[4].d1);
            //sprintf(ds, "%i", sell_dialog[3].d1);
            //ok(ds,"");

            //alfont_textprintf(vpage[page_num], font, 0, 10, MAKECOL_WHITE, "%i", sell_dialog[3].d1);
            dialog_message(sell_dialog, MSG_DRAW, 0, 0);
            if (!gui_update(sell_player)) {
                show_npc_dialog = shutdown_dialog(sell_player);
                sell_dialog[3].d1 = 0;
                if (show_npc_dialog == 1) {
                    WFIFOW(0) = net_w_value(0x00c9);
                    WFIFOW(2) = net_w_value(8);
                    WFIFOW(4) = net_w_value(get_item_index(sell_dialog[4].d1));
                    WFIFOW(6) = net_w_value(sell_dialog[3].d2);
                    WFIFOSET(8);
                }
                show_npc_dialog = 0;
                sell_dialog[3].d2 = 0;
                sprintf((char *)sell_dialog[5].dp, "%i", 0);
                sell_player = init_dialog(sell_dialog, -1);
                close_shop();
            }
            break;
        case 5:
            dialog_message(npc_list_dialog, MSG_DRAW, 0, 0);
            if (!gui_update(npc_list_player)) {
                show_npc_dialog = shutdown_dialog(npc_list_player);
                if (show_npc_dialog == 1) {
                    WFIFOW(0) = net_w_value(0x00b8);
                    WFIFOL(2) = net_l_value(current_npc);
                    WFIFOB(6) = net_b_value(npc_list_dialog[3].d1+1);
                    WFIFOSET(7);
                }
                show_npc_dialog = 0;
                npc_list_player = init_dialog(npc_list_dialog, -1);
                remove_all_items();
            }
            break;
    }

    if (show_skill_dialog) {
        update_skill_dialog();
        if (gui_update(skill_player) == 0) {
            show_skill_dialog = false;
        }
    }

    if (show_skill_list_dialog) {
        if (gui_update(skill_list_player) == 0) {
            int ret = shutdown_dialog(skill_list_player);
            if (ret == 1) {
                if (char_info->skill_point > 0) {
                    WFIFOW(0) = net_w_value(0x0112);
                    WFIFOW(2) = net_w_value(get_skill_id(skill_list_dialog[3].d1));
                    WFIFOSET(4);
                }
            } else if(ret == 2) {
                show_skill_list_dialog = false;
            }
            skill_list_player = init_dialog(skill_list_dialog, -1);
        }
    }

    draw_sprite(vpage[page_num], mouse_sprite, mouse_x, mouse_y);
    release_bitmap(vpage[page_num]);

    if (page_flipping) {
        show_video_bitmap(vpage[page_num]);
    } else {
        blit(vpage[page_num], screen, 0, 0, 0, 0, 800, 600);
    }
    page_num = 1 - page_num;
    gui_bitmap = vpage[page_num];
}

void exit_graphic() {
    shutdown_dialog(npc_player);
    shutdown_dialog(skill_player);
}

GraphicEngine::GraphicEngine() {
    tileset = new Spriteset("./data/graphic/tileset.dat");

    if (gfx_capabilities & GFX_HW_VRAM_BLIT) {
        BITMAP *page1 = create_video_bitmap(SCREEN_W, SCREEN_H);
        BITMAP *page2 = create_video_bitmap(SCREEN_W, SCREEN_H);
        if (page1 && page2) {
            surface = new VideoSurface(page1, page2);
        } else {
            if (page1)destroy_bitmap(page1);
            if (page2)destroy_bitmap(page2);
            BITMAP *buffer = create_bitmap(SCREEN_W, SCREEN_H);
            if (!buffer) {
                error("Not enough memory to create primary surface");
            }
            else {
                warning("Not enough video memory to create primary surface");
            }
            surface = new MemorySurface(buffer);
        }
    }
    else {
        BITMAP *buffer = create_bitmap(SCREEN_W, SCREEN_H);
        if (!buffer) {
            error("Not enough memory to create primary surface");
        }
        surface = new MemorySurface(buffer);
    }
}

GraphicEngine::~GraphicEngine() {
    delete tileset;
}
