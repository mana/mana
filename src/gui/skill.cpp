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

#include "skill.h"

extern CHAR_INFO  *char_info;

char str_string[8];
char agi_string[8];
char vit_string[8];
char int_string[8];
char dex_string[8];
char luk_string[8];

DIALOG skill_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,     300,  200,  120,   60,  0,    0,    0,    0,       0,                0,    (char *)"Skill",  NULL, NULL  },
   { tmw_text_proc,       304,  224,  252,  100,  0,    0,    0,    0,       0,                0,    str_string,       NULL, NULL  },
   { tmw_text_proc,       304,  234,  252,  100,  0,    0,    0,    0,       0,                0,    agi_string,       NULL, NULL  },
   { tmw_text_proc,       304,  244,  252,  100,  0,    0,    0,    0,       0,                0,    vit_string,       NULL, NULL  },
   { tmw_text_proc,       364,  224,  252,  100,  0,    0,    0,    0,       0,                0,    int_string,       NULL, NULL  },
   { tmw_text_proc,       364,  234,  252,  100,  0,    0,    0,    0,       0,                0,    dex_string,       NULL, NULL  },
   { tmw_text_proc,       364,  244,  252,  100,  0,    0,    0,    0,       0,                0,    luk_string,       NULL, NULL  },
   { NULL,                0,    0,    0,    0,    0,    0,    0,    0,       0,                0,    NULL,             NULL, NULL  }
};

void update_skill_dialog() {
	sprintf(str_string, "STR: %i", char_info->STR);
	sprintf(agi_string, "AGI: %i", char_info->AGI);
	sprintf(vit_string, "VIT: %i", char_info->VIT);
	sprintf(int_string, "INT: %i", char_info->INT);
	sprintf(dex_string, "DEX: %i", char_info->DEX);
	sprintf(luk_string, "LUK: %i", char_info->LUK);
}
