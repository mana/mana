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

#ifndef _GRAPHIC_H
#define _GRAPHIC_H

#include "../game.h"
#include "../map.h"
#include "../being.h"
#include "../gui/chat.h"
#include "../gui/inventory.h"
#include "../gui/shop.h"
#include "../gui/chat.h"
#include "../gui/inventory.h"
#include "../gui/npc.h"
#include "../../data/graphic/gfx_data.h"
#include "image.h"
#include <allegro.h>

extern BITMAP *buffer, *double_buffer;
extern char speech[255];
extern char npc_text[1000];
extern char skill_points[10];
extern Chat chatlog;
extern bool show_skill_dialog, show_skill_list_dialog;
extern int show_npc_dialog;
extern TmwInventory inventory;
extern int map_x, map_y, camera_x, camera_y;
extern BITMAP *hairset;
extern char npc_button[10];

// The action listener for the chat field
class ChatListener : public gcn::ActionListener {
    public:
        void action(const std::string& eventId);
};

void set_npc_dialog(int show);
void do_graphic(void);
void init_graphic(void);
void exit_graphic(void);

class Surface {
	public:
		BITMAP *buffer;
		virtual void lock() = 0;
		virtual void show() = 0;
		virtual void update() = 0;
};

class VideoSurface : public Surface {
	private:
		int	current_page;
		BITMAP *page[2];
	public:
		VideoSurface(BITMAP *page1, BITMAP *page2) {
			page[0] = page1;
			page[1] = page2;
			current_page = 0;
		}
		~VideoSurface() {
			destroy_bitmap(page[0]);
			destroy_bitmap(page[2]);
		}
		void lock() {
#ifdef WIN32
			acquire_bitmap(buffer);
#endif
		}
		void show() {
#ifdef WIN32
			release_bitmap(buffer);
#endif
			show_video_bitmap(buffer);
		}
		void update() {
			current_page++;
			if(current_page=2) {
				current_page = 0;
			}
			buffer = page[current_page];
		}
};

class MemorySurface : public Surface {
	public:
		MemorySurface(BITMAP *buffer) {
			this->buffer = buffer;
		}
		~MemorySurface() {
			destroy_bitmap(buffer);
		}
		void lock() {
		}
		void show() {
			blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
		}
		void update() {
		}
};		

class GraphicEngine {
	private:
		Surface *surface;
		Spriteset *tileset;
	public:
		GraphicEngine();
		~GraphicEngine();
		void refresh();	
};

#endif
