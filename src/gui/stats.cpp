/*

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

*/

#include "stats.h"

#include <iostream>

extern PLAYER_INFO  *char_info;

char stats_name[48];
char stats_hp[24];
char stats_sp[24];

DIALOG stats_dialog[] = {
	/* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)                    (d2)  (dp)              (dp2) (dp3) */
	{ tmw_dialog_proc,     493,  0,  300,   55,  0,    0,    0,    0,       0,                0,    stats_name,       NULL, NULL  },
	{ tmw_text_proc,       497,  22, 296,  100,  0,    0,    0,    0,       0,                0,    stats_hp,         NULL, NULL  },
	{ tmw_text_proc,       497,  34, 296,  100,  0,    0,    0,    0,       0,                0,    stats_sp,         NULL, NULL  },
	{ NULL,                0,    0,    0,    0,  0,    0,    0,    0,       0,                0,    NULL,             NULL, NULL  }
};

void update_stats_dialog() {
	sprintf(stats_name, "%s Lvl:%i Job:%i", char_info->name, char_info->lv, char_info->job_lv);
	sprintf(stats_hp, "HP %i / %i", char_info->hp, char_info->max_hp);
	sprintf(stats_sp, "SP %i / %i", char_info->sp, char_info->max_sp);
}

/*
typedef struct {
	int id;
	char name[24];
	short hp, max_hp, sp, max_sp, lv;
	int xp, zeny, job_xp, job_lv;
	short statp, skillp;
	char STR, AGI, VIT, INT, DEX, LUK;
} PLAYER_INFO;
*/
