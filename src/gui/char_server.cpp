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

#include "char_server.h"
#include "../graphic/graphic.h"

char server[30];

DIALOG char_server_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)                          (dp2) (dp3) */
   { tmw_dialog_proc,   300,  240,  200,  104,    0,  -1,    0,    0,          0,                 0, (char*)"Server select",                     NULL, NULL  },
   { tmw_list_proc,     304,  262,  192,   55,    0,   0,    0,    0,          0,                 0,     (char*)server_list,                     NULL, NULL  },
   { tmw_button_proc,   398,  322,  44,    18,    0,  -1,    'o',  D_EXIT,    -1,                 0,           (char*)"&Ok",                     NULL, NULL  },
   { tmw_button_proc,   446,  322,  44,    18,    0,  -1,    'c',  D_EXIT,    -1,                 0,       (char*)"&Cancel",                     NULL, NULL  },
   { NULL,                0,    0,   0,     0,    0,   0,    0,    0,          0,                 0,                   NULL,                     NULL, NULL  },
};

/** Helper function to create server list */
char *server_list(int index, int *size) {
	static char buffer[30];
    if (index < 0) {
        *size = n_server;
        return NULL;
	}
	sprintf(buffer, "%s (%i)", server_info[index].name, server_info[index].online_users);
	return buffer;
}

void char_server() {
	state = LOGIN;
	centre_dialog(char_server_dialog);
	DIALOG_PLAYER *player = init_dialog(char_server_dialog, -1);
	if(!player)ok("Error", "Unable to initialize login dialog");
	int gui_exit = 1;
	if(n_server==0)char_server_dialog[2].flags |= D_DISABLED;
	while ((!key[KEY_ESC])&&(gui_exit)&&(!key[KEY_ENTER])) {
		clear_bitmap(buffer);
		blit((BITMAP *)graphic[LOGIN_BMP].dat, buffer, 0, 0, 0, 0, 800, 600);
		gui_exit = gui_update(player);
		blit(buffer, screen, 0, 0, 0, 0, 800, 600);
	}
	gui_exit = shutdown_dialog(player);
	if((gui_exit==2)||(key[KEY_ENTER])) {
		server_char_server();
	}
}

void server_char_server() {
	int ret;
	state = LOGIN;

    // Connect to char server
	ret = open_session(iptostring(server_info[char_server_dialog[1].d1].address), server_info[char_server_dialog[1].d1].port);
	if(ret==SOCKET_ERROR) {
		ok("Error", "Unable to connect to char server");
		return;
	}

	// Send login infos
	WFIFOW(0) = net_w_value(0x0065);
	WFIFOL(2) = net_l_value(account_ID);
	WFIFOL(6) = net_l_value(session_ID1);
	WFIFOL(10) = net_l_value(session_ID2);
	WFIFOW(14) = 0;
	WFIFOB(16) = net_b_value(sex);
	WFIFOSET(17);

	while((in_size<4)||(out_size>0))flush();
	RFIFOSKIP(4);

    while(in_size<3)flush();

	if(RFIFOW(0)==0x006b) {
		while(in_size<RFIFOW(2))flush();
		n_character = (RFIFOW(2)-24)/106;
		char_info = (PLAYER_INFO *)malloc(sizeof(PLAYER_INFO)*n_character);
		for(int i=0;i<n_character;i++) {
			char_info[i].id = RFIFOL(24+106*i);
			strcpy(char_info[i].name, RFIFOP(24+106*i+74));
			char_info[i].hp = RFIFOW(24+106*i+42);
			char_info[i].max_hp = RFIFOW(24+106*i+44);
			char_info[i].xp = RFIFOL(24+106*i+4);
			char_info[i].gp = RFIFOL(24+106*i+8);
			char_info[i].job_xp = RFIFOL(24+106*i+12);
			char_info[i].job_lv = RFIFOL(24+106*i+16);
			char_info[i].sp = RFIFOW(24+106*i+46);
			char_info[i].max_sp = RFIFOW(24+106*i+48);
			char_info[i].lv = RFIFOW(24+106*i+58);
			char_info[i].STR = RFIFOB(24+106*i+98);
			char_info[i].AGI = RFIFOB(24+106*i+99);
			char_info[i].VIT = RFIFOB(24+106*i+100);
			char_info[i].INT = RFIFOB(24+106*i+101);
			char_info[i].DEX = RFIFOB(24+106*i+102);
			char_info[i].LUK = RFIFOB(24+106*i+103);
			char_info[i].hair_style = RFIFOW(24+106*i+54);
			char_info[i].hair_color = RFIFOW(24+106*i+70);
			char_info[i].weapon = RFIFOW(24+106*i+56);
		}
		state = CHAR_SELECT;

		log("Player", "name", char_info->name);
		log_hex("Char_Server_Packet", "Packet_ID", RFIFOW(0));
    log_int("Char_Server_Packet", "Packet_length", RFIFOW(2));


		RFIFOSKIP(RFIFOW(2));
	} else if(RFIFOW(0)==0x006c) {
		switch(RFIFOB(2)) {
			case 0:
				ok("Error", "Access denied");
				break;
			case 1:
				ok("Error", "Cannot use this ID");
                break;
			default:
				ok("Error", "Rejected from server");
				break;
		}
		RFIFOSKIP(3);
		close_session();
	} else ok("Error", "Unknown error");
	// Todo: add other packets
}
