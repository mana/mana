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

char stats_name[42];
char stats_hp[24];
char stats_sp[24];
char stats_gp[24];

DIALOG stats_dialog[] = {
	/* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)        (dp2) (dp3) */
	{ tmw_dialog_proc,    493-10,  0+10,  300,   55,  0,    0,    0,    0,       0,    0,    stats_name, NULL, NULL  },
	{ tmw_text_proc,       497-10,  34+10, 296,  100,  0,    0,    0,    0,       0,    0,    stats_hp,   NULL, NULL  },
	{ tmw_bar_proc,        507-10,  22+10,  60,    18,    0,   0,    '1',  0,          1,                      1,				NULL,         NULL, NULL  }, 
	{ tmw_text_proc,       707-10,  34+10, 296,  100,  0,    0,    0,    0,       0,    0,    stats_gp, NULL, NULL  },
	{ tmw_text_proc,       607-10,  34+10, 296,  100,  0,    0,    0,    0,       0,    0,    stats_sp,   NULL, NULL  },
	{ tmw_bar_proc,        617-10,  22+10,  60,    18,    0,   0,    '1',  0,          1,                      1,				NULL,         NULL, NULL  }, 
	{ NULL,                0,    0,    0,    0,  0,    0,    0,    0,       0,    0,    NULL,       NULL, NULL  }
};

/**
	updates stats_dialog w/ values from PLAYER_INFO *char_info
*/
void update_stats_dialog() {
	sprintf(stats_name, "%s Lvl: % 2i Job: % 2i", char_info->name, char_info->lv, char_info->job_lv);
	sprintf(stats_hp,   "HP % 4d / % 4d",         char_info->hp, char_info->max_hp);
	sprintf(stats_gp, "GP % 5i",                char_info->gp);
	sprintf(stats_sp,   "SP % 4d / % 4d",         char_info->sp, char_info->max_sp);
	stats_dialog[2].d1 = char_info->hp;
	stats_dialog[2].d2 = char_info->max_hp;
}
