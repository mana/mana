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

#include "skill.h"

extern PLAYER_INFO  *char_info;

int n_skills = 0;
SKILL *skill_head = NULL;

char str_string[8];
char agi_string[8];
char vit_string[8];
char int_string[8];
char dex_string[8];
char luk_string[8];

char *skill_db[] = {
	// 0-99
	"", "Basic", "", "", "", "", "", "", "", "",
  "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	// 100-199
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "First aid", "Play as dead", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "",
};

DIALOG skill_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,     300,  200,  150,   60,  0,    0,    0,    0,       0,                0,    (char *)"Skills",  NULL, NULL  },
   { tmw_text_proc,       304,  224,  252,  100,  0,    0,    0,    0,       0,                0,    str_string,       NULL, NULL  },
   { tmw_plus_proc,       354,  224,  16,    16,    0,   0,    '0',  0,          0,			   1,     (void*)increaseStatus,NULL, NULL  },
   { tmw_text_proc,       304,  234,  252,  100,  0,    0,    0,    0,       0,                0,    agi_string,       NULL, NULL  },
   { tmw_plus_proc,       354,  234,  16,    16,    0,   0,    '0',  0,          1,			   1,     (void*)increaseStatus,NULL, NULL  },
   { tmw_text_proc,       304,  244,  252,  100,  0,    0,    0,    0,       0,                0,    vit_string,       NULL, NULL  },
   { tmw_plus_proc,       354,  244,  16,    16,    0,   0,    '0',  0,          2,			   1,     (void*)increaseStatus,NULL, NULL  },
   { tmw_text_proc,       374,  224,  252,  100,  0,    0,    0,    0,       0,                0,    int_string,       NULL, NULL  },
   { tmw_plus_proc,       424,  224,  16,    16,    0,   0,    '0',  0,          3,			   1,     (void*)increaseStatus,NULL, NULL  },
   { tmw_text_proc,       374,  234,  252,  100,  0,    0,    0,    0,       0,                0,    dex_string,       NULL, NULL  },
   { tmw_plus_proc,       424,  234,  16,    16,    0,   0,    '0',  0,          4,			   1,     (void*)increaseStatus,NULL, NULL  },
   { tmw_text_proc,       374,  244,  252,  100,  0,    0,    0,    0,       0,                0,    luk_string,       NULL, NULL  },
   { tmw_plus_proc,       424,  244,  16,    16,    0,   0,    '0',  0,          5,			   1,     (void*)increaseStatus,NULL, NULL  },
   { NULL,                0,    0,    0,    0,    0,    0,    0,    0,       0,                0,    NULL,             NULL, NULL  }
};

void update_skill_dialog() {
	int skillTemp = 0;
	for(int loop = 0; loop<=char_info->lv;loop++)
		skillTemp += int( (float)loop / (float)5.0 ) + (int)2;
	sprintf(str_string, "STR: %i", char_info->STR);
	sprintf(agi_string, "AGI: %i", char_info->AGI);
	sprintf(vit_string, "VIT: %i", char_info->VIT);
	sprintf(int_string, "INT: %i", char_info->INT);
	sprintf(dex_string, "DEX: %i", char_info->DEX);
	sprintf(luk_string, "LUK: %i", char_info->LUK);
	if(char_info->STR+char_info->AGI+char_info->VIT+char_info->INT+char_info->DEX+char_info->LUK == skillTemp)
		{
		skill_dialog[2].d2 = skill_dialog[4].d2 = skill_dialog[6].d2 =skill_dialog[8].d2 =skill_dialog[10].d2 = 0;
		} else {
		skill_dialog[2].d2 = skill_dialog[4].d2 = skill_dialog[6].d2 =skill_dialog[8].d2 =skill_dialog[10].d2 = 1;
		}
}

void add_skill(short id, short lv, short sp) {
	SKILL *skill = skill_head;
	SKILL *temp = (SKILL *)malloc(sizeof(SKILL));
	temp->id = id;
	temp->lv = lv;
	temp->sp = sp;
	temp->next = NULL;
	if(!skill_head)
		skill_head = temp;
	else {
		while(skill->next)
			skill = skill->next;
		skill->next = temp;
	}
}

char *skill_list(int index, int *list_size) {
	if(index<0) {
    *list_size = n_skills;
		return NULL;
	} else {
		int iterator = 0;
		SKILL *temp = skill_head;
		while(iterator<index) {
			temp = temp->next;
			iterator++;
		}
		char *name = (char *)malloc(30);
		sprintf(name, "%s lv:%i %i SP", skill_db[temp->id], temp->lv, temp->sp);
		return name;
		// need to clean allocated memory
	}
}

int get_skill_id(int index) {
	int iterator = 0;
  SKILL *temp = skill_head;
  while(iterator<index) {
    temp = temp->next;
    iterator++;
  }
  return temp->id;
}

SKILL *is_skill(int id) {
	SKILL *temp = skill_head;
	while(temp) {
		if(temp->id==id)return temp;
		temp = temp->next;
	}
	return NULL;
}

void increaseStatus(void *dp3, int d1) {
	WFIFOW(0) = net_w_value(0x00bb);
	switch(d1) {
	case 0:
		WFIFOW(2) = net_w_value(0x000d);
		break;
	case 1:
		WFIFOW(2) = net_w_value(0x000e);
		break;
	case 2:
		WFIFOW(2) = net_w_value(0x000f);
		break;
	case 3:
		WFIFOW(2) = net_w_value(0x0010);
		break;
	case 4:
		WFIFOW(2) = net_w_value(0x0011);
		break;
	case 5:
		WFIFOW(2) = net_w_value(0x0012);
		break;
	}
	WFIFOW(4) = net_b_value(1);
	WFIFOSET(5);
	while((out_size>0))flush();
	skill_dialog[2].d2 = skill_dialog[4].d2 = skill_dialog[6].d2 = skill_dialog[8].d2 = skill_dialog[10].d2 = 0;
}
