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

#ifndef _GRAPHIC_H
#define _GRAPHIC_H

#ifdef WIN32
#pragma warning (disable:4312)
#endif

#include <allegro.h>
#include "../Gui/chat.h"
#include "../Gui/inventory.h"

extern BITMAP *buffer, *double_buffer;
extern char speech[255];
extern char npc_text[1000];
extern Chat chatlog;
extern bool show_skill_dialog;
extern int show_npc_dialog;
extern TmwInventory inventory;
extern int map_x, map_y;

void do_graphic(void);
void init_graphic(void);
void exit_graphic(void);

#endif
