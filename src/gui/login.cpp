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

*/

#include "login.h"
#include "../graphic/graphic.h"

/** Display login GUI */
void login() {
DIALOG login_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)              (dp2) (dp3) */
   { tmw_dialog_proc,   300,  252,  200,   96,    0,  -1,    0,    0,          0,                      0,    (char*)"Login",         NULL, NULL  },
   { tmw_text_proc,     304,  284,  50,    10,    0,   0,    0,    0,          0,                      0,    (char*)"Name:",         NULL, NULL  },
   { tmw_text_proc,     304,  304,  50,    10,    0,   0,    0,    0,          0,                      0,(char*)"Password:",         NULL, NULL  },
   { tmw_edit_proc,     360,  280,  130,   18,    0,  -1,    0,    0,          24,                     0,          username,         NULL, NULL  },
   { tmw_password_proc, 360,  300,  130,   18,    0,  -1,    0,    0,          24,                     0,          password,         NULL, NULL  },
   { tmw_button_proc,   398,  322,  44,    18,    0,  -1,    'o',  D_EXIT,    -1,                      0,      (char*)"&Ok",         NULL, NULL  },
   { tmw_button_proc,   446,  322,  44,    18,    0,  -1,    'c',  D_EXIT,    -1,                      0,  (char*)"&Cancel",         NULL, NULL  },
   { tmw_check_proc,    304,  322,  60,    18,    0,   0,    '1',  0,          0,                      0,     (char*)"keep",         NULL, NULL  }, 
   { NULL,                0,    0,   0,     0,    0,   0,    0,    0,          0,                      0,              NULL,         NULL, NULL  },
};

	if(get_config_int("login", "remember", 0)!=0) {
		login_dialog[7].flags = D_SELECTED;
		if(get_config_string("login", "username", 0))strcpy(username, get_config_string("login", "username", 0));
        else strcpy(username, "player\0");
	}
	centre_dialog(login_dialog);
	DIALOG_PLAYER *player = init_dialog(login_dialog, -1);
	int gui_exit = 1;
	while ((!key[KEY_ESC])&&(gui_exit)&&(state!=EXIT)&&(!key[KEY_ENTER])) {
		clear_bitmap(buffer);
		if(stretch_mode!=0)blit((BITMAP *)graphic[LOGIN_BMP].dat, buffer, 0, 0, 80, 60, 640, 480);
		else blit((BITMAP *)graphic[LOGIN_BMP].dat, buffer, 0, 0, -120, -90, 640, 480);

		gui_exit = gui_update(player);
		blit(buffer, screen, 0, 0, 0, 0, 800, 600);
	}
	state = EXIT;
	set_config_int("login", "remember", (login_dialog[7].flags & D_SELECTED)>>1);
	if(login_dialog[7].flags & D_SELECTED) {
    if(!username)strcpy(username, "player\0");
    set_config_string("login", "username", username);
  } else set_config_string("login", "username", "player\0");
	log("Player", "username", username);
  gui_exit = shutdown_dialog(player);
	if((gui_exit==5)||(key[KEY_ENTER])) {
        if(username[0]=='\0') {
            ok("Error", "Enter your username first");
            warning("Enter your username first");
            state = LOGIN;
        } else {
            server_login();
            close_session();
        }
	}
}

/** Attempt to login to login server */
void server_login() {
	int ret;    

    // Connect to login server
	ret = open_session(get_config_string("server", "host", 0), get_config_int("server", "port", 0));
	if(ret==SOCKET_ERROR) {
		state = LOGIN;
		ok("Error", "Unable to connect to login server");
		warning("Unable to connect to login server");
		return;
	}
	
	// Send login infos
	

	WFIFOW(0) = net_w_value(0x0064);
	
	WFIFOL(2) = 0;
	memcpy(WFIFOP(6), username, 24);
	memcpy(WFIFOP(30), password, 24);
	WFIFOB(54) = 0;
	WFIFOSET(55);

	while((in_size<23)||(out_size>0))flush();
	log_hex("Login_Packet", "Packet_ID", RFIFOW(0));
    log_int("Login_Packet", "Packet_length", get_packet_length(RFIFOW(0)));
		
	if(RFIFOW(0)==0x0069) {
		while(in_size<RFIFOW(2))flush();
		n_server = (RFIFOW(2)-47)/32;
		server_info = (SERVER_INFO *)malloc(sizeof(SERVER_INFO)*n_server);
		account_ID = RFIFOL(8);
		session_ID1 = RFIFOL(4);
		session_ID2 = RFIFOL(12);
		sex = RFIFOB(46);
		for(int i=0;i<n_server;i++) {
			server_info[i].address = RFIFOL(47+32*i);
			memcpy(server_info[i].name, RFIFOP(47+32*i+6), 20);
			server_info[i].online_users = RFIFOW(47+32*i+26);
			server_info[i].port = RFIFOW(47+32*i+4);
			state = CHAR_SERVER;
		}
		log("Login_Packet", "server_address", iptostring(server_info[0].address));
		log("Login_Packet", "server_name", server_info[0].name);
		log_int("Login_Packet", "server_users", server_info[0].online_users);
		log_int("Login_Packet", "server_port", server_info[0].port);
		RFIFOSKIP(RFIFOW(2));
	} else if(RFIFOW(0)==0x006a) {
		switch(RFIFOB(2)) {
			case 0:
				ok("Error", "Unregistered ID");
				break;
			case 1:
				ok("Error", "Wrong password");
				break;
		}
		state = LOGIN;
		RFIFOSKIP(23);
	} else ok("Error", "Unknown error");
	// Todo: add other packets, also encrypted
}
