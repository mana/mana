/**

	The Mana World
	Copyright 2004 The Mana World Development Team

    This file is part of The Mana World.

    The Mana World is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    The Mana World is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Mana World; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

		By ElvenProgrammer aka Eugenio Favalli (umperio@users.sourceforge.net)

*/

#include "graphic.h"

#define TILESET_W 480
#define TILESET_H 320

#ifdef WIN32
#pragma warning (disable:4312)
#endif

#include <allegro.h>
#include "../game.h"
#include "../map.h"
#include "../being.h"
#include "../gui/chat.h"
#include "../gui/inventory.h"
#include "../gui/shop.h"
#include "../../data/graphic/gfx_data.h"

BITMAP *buffer, *double_buffer, *chat_background;
DATAFILE *tileset;
//extern char* itemCurrenyQ;
char itemCurrenyQ[10] = "0";
char page_num;
int map_x, map_y, camera_x, camera_y;
DIALOG_PLAYER *chat_player, *npc_player, *skill_player, *buy_sell_player, *buy_player, *sell_player;
char speech[255] = "";
char npc_text[1000] = "";
TmwInventory inventory;
Chat chatlog("./docs/chatlog.txt", 20);
int show_npc_dialog = 0;
bool show_skill_dialog = false;

DIALOG npc_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,     300,  200,  260,  150,  0,    0,    0,    0,       0,                0,    (char *)"NPC",             NULL, NULL  },
   { tmw_button_proc,     508,  326,  50,   20,   255,  0,    'c',  D_EXIT,  0,                0,    (char *)"&Close",         NULL, NULL  },
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
   { tmw_dialog_proc,     300,  200,  260,  150,  0,    0,    0,    0,       0,                0,    (char *)"Buy",     NULL, NULL  },
   { tmw_button_proc,     450,  326,  50,   20,   255,  0,    'o',  D_EXIT,  0,                0,    (char *)"&Ok",     NULL, NULL  },
	 { tmw_button_proc,     508,  326,  50,   20,   255,  0,    'c',  D_EXIT,  0,                0,    (char *)"&Cancel", NULL, NULL  },
   { tmw_list_proc,       304,  224,  252,  100,  0,    0,    0,    0,       0,                0,    (char *)shop_list, NULL, NULL  },
	 { tmw_text_proc,       304,  326,  180,  100,  0,    0,    0,    0,       0,                0,    NULL,              NULL, NULL  },
   { NULL,                0,    0,    0,    0,    0,    0,    0,    0,       0,                0,    NULL,              NULL, NULL  }
};

DIALOG sell_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,     300,  200,  260,  175,  0,    0,    0,    0,       0,                0,    (char *)"Sell",    NULL, NULL  },
   { tmw_button_proc,     450,  326+25,  50,   20,   255,  0,    'o',  D_EXIT,  0,             0,    (char *)"&Ok",     NULL, NULL  },
   { tmw_button_proc,     508,  326+25,  50,   20,   255,  0,    'c',  D_EXIT,  0,			   0,    (char *)"&Cancel", NULL, NULL  },
   { tmw_slider_proc,     304,  326,  200,   20,   255,  0,    0,  0,		10,				   0,	 NULL,   (void *)changeQ, NULL  },
   { tmw_list_proc,       304,  224,  252,  100,  0,    0,    0,    0,       0,                0,    (char *)shop_list, NULL, NULL  },
   { tmw_text_proc,       514,  326,  40,  100,  0,    0,    0,    0,       0,                0,    (char *)itemCurrenyQ, NULL, NULL  },
   { NULL,                0,    0,    0,    0,    0,    0,    0,    0,       0,                0,    NULL,              NULL, NULL  }
};
 
DIALOG chat_dialog[] = {
   /* (dialog proc)         (x)   (y)   (w)  (h) (fg) (bg)  (key) (flags)   (d1)               (d2)           (dp)         (dp2) (dp3) */
   { tmw_edit_proc,          0,  574,  592,  25,  0,    0,    'c',  0,       90,               0,           speech,        NULL, NULL  },
   { NULL,                   0,    0,    0,   0,  0,    0,    0,    0,       0,                0,             NULL,        NULL, NULL  }
};

int get_x_offset(NODE *node) {
	int offset = 0;
	char direction = get_direction(node->coordinates);
	if(node->action==WALK) {
		if(direction==WEST || direction==EAST) {
      offset = node->frame + 1;
      if(offset==5)offset = 0;
      offset *= 4;
			if(direction==WEST) {
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
	if(node->action==WALK) {
		if(direction==SOUTH || direction==NORTH) {
      offset = node->frame + 1;
      if(offset==5)offset = 0;
      offset *= 4;
			if(direction==NORTH) {
				offset = -offset;
				offset += 16;
			} else offset -= 16;
		}
	}
	return offset;
}

void init_graphic() {
	tileset = load_datafile("./data/graphic/desert.dat");
	if(!tileset)error("Unable to load tileset datafile");
	//if(!(gfx_capabilities & GFX_HW_VRAM_BLIT))allegro_message("Not supporting HW accelerated blit");
	buffer = create_bitmap(SCREEN_W/2, SCREEN_H/2);
	double_buffer = create_bitmap(SCREEN_W, SCREEN_H);

  alfont_set_font_size(gui_font, 16);
	clear_bitmap(screen);
  chat_background = create_bitmap(592, 100);
  clear_to_color(chat_background, makecol(0,0,0));

	// Initialize gui
  chat_player = init_dialog(chat_dialog, -1);
  npc_player = init_dialog(npc_dialog, -1);
	position_dialog(npc_dialog, 300, 200);
	skill_player = init_dialog(skill_dialog, -1);
	buy_sell_player = init_dialog(buy_sell_dialog, -1);
	buy_player = init_dialog(buy_dialog, -1);
	sell_player = init_dialog(sell_dialog, -1);
  gui_bitmap = double_buffer;
	alfont_text_mode(-1);
	inventory.create(100, 100);
}

void do_graphic(void) {

	map_x = (get_x(player_node->coordinates)-13)*16+get_x_offset(player_node);
	map_y = (get_y(player_node->coordinates)-9)*16+get_y_offset(player_node);

  camera_x = map_x >> 4;
	camera_y = map_y >> 4;

  int offset_x = map_x & 15;
	int offset_y = map_y & 15;

	sort();
	
	
	for(int j=0;j<20;j++)
		for(int i=0;i<26;i++) {
			if( /* get_tile(i+camera_x, j+camera_y, 0) >= 0 && */  get_tile(i+camera_x, j+camera_y, 0) < 600)
				draw_rle_sprite(buffer, (RLE_SPRITE *)tileset[get_tile(i+camera_x, j+camera_y, 0)].dat, i*16-offset_x, j*16-offset_y);
			if( get_tile(i+camera_x, j+camera_y, 1) > 0 && get_tile(i+camera_x, j+camera_y, 1)<600)
				draw_rle_sprite(buffer, (RLE_SPRITE *)tileset[get_tile(i+camera_x, j+camera_y, 1)].dat, i*16-offset_x, j*16-offset_y);
		}

	NODE *node = get_head();
	NODE *old_node = NULL;
	while(node) {
    if((node->job>=100)&&(node->job<=110)) { // Draw a NPC
			masked_blit((BITMAP *)graphic[NPCSET_BMP].dat, buffer, (get_direction(node->coordinates)/2+4*(node->job-100))*25, 0, (get_x(node->coordinates)-camera_x)*16-4-offset_x, (get_y(node->coordinates)-camera_y)*16-24-offset_y, 25, 40);  
		} else if(node->job<10) { // Draw a player
			node->text_x = (get_x(node->coordinates)-camera_x)*16-34+get_x_offset(node)-offset_x;
			node->text_y = (get_y(node->coordinates)-camera_y)*16-36+get_y_offset(node)-offset_y;
			masked_blit((BITMAP *)graphic[PLAYERSET_BMP].dat, buffer, 80*(get_direction(node->coordinates)/2), 60*(node->frame+node->action), node->text_x, node->text_y, 80, 60);

			if(node->emotion!=0) {
        draw_sprite(buffer, (BITMAP *)emotions[node->emotion-1].dat, (get_x(node->coordinates)-camera_x)*16-5+get_x_offset(node)-offset_x, (get_y(node->coordinates)-camera_y)*16-45+get_y_offset(node)-offset_y);
        node->emotion_time--;
        if(node->emotion_time==0)
          node->emotion = 0;
			}

			if(node->action!=STAND && node->action!=SIT) {
				node->frame = (get_elapsed_time(node->tick_time)*4)/(node->speed);
	      if(node->frame>=4) {
          node->frame = 0;
					node->action = STAND;
//					node->tick_time;
					if(node->id==player_node->id)
            walk_status = 0;          
				}
			}

		} else if(node->job==45) { // Draw a warp
      //rectfill(buffer, (get_x(node->coordinates)-map_x)*16-player_x-get_x_offset(node->frame, get_direction(node->coordinates)), (get_y(node->coordinates)-map_y)*16-player_y-get_y_offset(node->frame, get_direction(node->coordinates)), (get_x(node->coordinates)-map_x)*16-player_x-get_x_offset(node->frame, get_direction(node->coordinates))+16, (get_y(node->coordinates)-map_y)*16-player_y-get_y_offset(node->frame, get_direction(node->coordinates))+16, makecol(0,0,255));
    } else { // Draw a monster

			node->text_x = (get_x(node->coordinates)-camera_x)*16-20+get_x_offset(node)-offset_x;
			node->text_y = (get_y(node->coordinates)-camera_y)*16-25+get_y_offset(node)-offset_y;

      if(node->action==MONSTER_DEAD)node->frame = 0;
			masked_blit((BITMAP *)graphic[MOBSET_BMP].dat, buffer, (get_direction(node->coordinates)/2)*60+240*(node->job-1002), 60*(node->frame+node->action), node->text_x, node->text_y, 60, 60);
      if(node->action!=STAND) {
        node->frame = (get_elapsed_time(node->tick_time)*4)/(node->speed);
        if(node->frame>=4) {
          if(node->action!=MONSTER_DEAD) {
            if(node->path  && node->action!=MONSTER_DEAD) {
              PATH_NODE *old = node->path;
              set_coordinates(node->coordinates, node->path->x, node->path->y, 0);
              node->path = node->path->next;
              if(old!=NULL)
                free(old);
            } else node->action = STAND;
            if(node->action!=MONSTER_DEAD)node->frame = 0;
            node->tick_time = tick_time;
					}
        }
      }
    }
		old_node = node;
    node = node->next;
		if(old_node->action==MONSTER_DEAD && old_node->frame>=4)
			remove_node(old_node->id);
	}

	for(int j=0;j<20;j++)
		for(int i=0;i<26;i++) {
			if(get_tile(i+camera_x, j+camera_y, 2)>0 && get_tile(i+camera_x, j+camera_y, 2)<600)draw_rle_sprite(buffer, (RLE_SPRITE *)tileset[get_tile(i+camera_x, j+camera_y, 2)].dat, i*16-offset_x, j*16-offset_y);
		}

	stretch_blit(buffer, double_buffer, 0, 0, 400, 300, 0, 0, 800, 600);

	// Draw player speech
  node = get_head();
  while(node) {
		if(node->speech!=NULL) {
			if(node->speech_color==makecol(255,255,255))
        alfont_textprintf_aa(double_buffer, gui_font, node->text_x*2+90-alfont_text_length(gui_font, node->speech)/2, node->text_y*2, node->speech_color, "%s", node->speech);
			else
				alfont_textprintf_aa(double_buffer, gui_font, node->text_x*2+60-alfont_text_length(gui_font, node->speech)/2, node->text_y*2, node->speech_color, "%s", node->speech);
			      
      node->speech_time--;
      if(node->speech_time==0) {
        free(node->speech);
        node->speech = NULL;
      }
    }
    node = node->next;
  }	

	inventory.draw(double_buffer);

	set_trans_blender(0, 0, 0, 110);
  draw_trans_sprite(double_buffer, chat_background, 0, SCREEN_H-125);
  
  chatlog.chat_draw(double_buffer, 8, gui_font);
  gui_update(chat_player);

	switch(show_npc_dialog) {
		case 1:
      dialog_message(npc_dialog, MSG_DRAW, 0, 0);
      if(!(show_npc_dialog = gui_update(npc_player)))strcpy(npc_text, "");
			break;
		case 2:
			dialog_message(buy_sell_dialog, MSG_DRAW, 0, 0);
			if(!gui_update(buy_sell_player)) {
				show_npc_dialog = shutdown_dialog(buy_sell_player);
				if(show_npc_dialog==1 || show_npc_dialog==2) {
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
			sprintf(money, "%i gp", char_info->zeny);
			buy_dialog[4].dp = &money;
			dialog_message(buy_dialog, MSG_DRAW, 0, 0);
			if(!gui_update(buy_player)) {
				show_npc_dialog = shutdown_dialog(buy_player);
        if(show_npc_dialog==1) {
          WFIFOW(0) = net_w_value(0x00c8);
          WFIFOW(2) = net_w_value(8);
          WFIFOW(4) = net_w_value(1);
          WFIFOW(6) = net_w_value(get_item_id(buy_dialog[3].d1));
          WFIFOSET(8);
				}
				show_npc_dialog = 0;
        buy_player = init_dialog(buy_dialog, -1);
				close_shop();
			}
			break;
		case 4:
      sell_dialog[3].d1 = get_item_quantity(sell_dialog[4].d1);
			dialog_message(sell_dialog, MSG_DRAW, 0, 0);
			if(!gui_update(sell_player)) {
				show_npc_dialog = shutdown_dialog(sell_player);
				
        if(show_npc_dialog==1) {
          WFIFOW(0) = net_w_value(0x00c9);
          WFIFOW(2) = net_w_value(8);
          WFIFOW(4) = net_w_value(get_item_index(sell_dialog[4].d1));
          WFIFOW(6) = net_w_value(sell_dialog[3].d2);
          WFIFOSET(8);
				}
				show_npc_dialog = 0;
        sell_player = init_dialog(sell_dialog, -1);
				close_shop();
			}
			break;
  }

	if(show_skill_dialog) {
		update_skill_dialog();
    if(gui_update(skill_player)==0)show_skill_dialog = false;
	}
  
	alfont_textprintf(double_buffer, gui_font, 0, 0, MAKECOL_WHITE, "FPS:%i", fps);

	blit(double_buffer, screen, 0, 0, 0, 0, 800, 600);

	frame++;
}

void exit_graphic() {
	shutdown_dialog(npc_player);
  shutdown_dialog(chat_player);
	shutdown_dialog(skill_player);
}
