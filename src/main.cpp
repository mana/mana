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

#include "main.h"
#include "./sound/sound.h"
#include "./graphic/graphic.h"

#include <iostream>

/* Account infos */
int account_ID, session_ID1, session_ID2;
char sex, n_server, n_character;
SERVER_INFO *server_info;
PLAYER_INFO *char_info = new PLAYER_INFO;

BITMAP *playerset;
DATAFILE *graphic, *emotions;

char username[25];
char password[25];
int map_address, char_ID;
short map_port;
char map_name[16];
unsigned char state;
unsigned short x, y;
unsigned char direction;
unsigned short job, hair, hair_color;
unsigned char stretch_mode;

// new sound-engine /- kth5
TmwSound sound;

void request_exit() {
  state = EXIT;
}

/** Do all initialization stuff */
void init_engine() {
	allegro_init();
  init_log();
  set_close_button_callback(request_exit);  // we should not use set_window_close_hook() since it's deprecated and might be gone in the future /-kth5
  set_config_file("tmw.ini");
  #ifdef MACOSX
  set_color_depth(32);
  Init_SuperEagle(32);
  #else
  set_color_depth(16);
  Init_SuperEagle(16);
  #endif
	stretch_mode = get_config_int("settings", "stretch", 0);
	if(stretch_mode==0) {
    if(set_gfx_mode(get_config_int("settings", "screen", 0), 400, 300, 0, 0))
      error(allegro_error);
	} else {
		if(set_gfx_mode(get_config_int("settings", "screen", 0), 800, 600, 0, 0))
      error(allegro_error);
	}
  if(install_keyboard())
    error("Unable to install keyboard");
  if(install_timer())
    error("Unable to install timer");
  if(install_mouse()==-1)
    error("Unable to install mouse");
  if(alfont_init()!=ALFONT_OK)
    error("Unable to install AllegroFont");

  buffer = create_bitmap(800, 600);
  if(!buffer)
    error("Not enough memory to create buffer");

  graphic = load_datafile("./data/graphic/graphic.dat");
  if(graphic==NULL)
    error("Unable to load graphic datafile");
	playerset = (BITMAP*)graphic[PLAYERSET_BMP].dat;
	emotions = load_datafile("./data/graphic/emotions.dat");
	if(emotions==NULL)
		error("Unable to load emotions datafile");

	init_gui(buffer, "./data/Skin/aqua.skin");
  state = LOGIN;
}

/** Clear the engine */
void exit_engine() {
  gui_exit();
  //alfont_exit();
  destroy_bitmap(buffer);
  allegro_exit();
}

/** Main */
int main() {
	init_engine();
	// initialize sound-engine and start playing intro-theme /-kth5
	try{
		if(get_config_int("settings", "sound", 0)==1)
		sound.Init(32,20);                          // inits the sound-subsystem w/ 32 voices / 20 for mod
		sound.SetVol(128,128,128);                    // sets intial volume parameters
		//#ifdef WIN32
			//sound.StartMIDI("Sound/Midis/city.mid",-1);   // play a midi file
		//#endif
		//sound.LoadItem("test.wav", TMWSOUND_SFX);
	}catch(const char * err){                       // catch errors and show appropriate messages on-screen (elven plz... ^^)
		ok("Sound Engine", err);
		warning(err);
	}

  while(state!=EXIT) {
    switch(state) {
      case LOGIN:
        status("LOGIN");
        login();
        break;
      case CHAR_SERVER:
        status("CHAR_SERVER");
        char_server();
        break;
      case CHAR_SELECT:
        status("CHAR_SELECT");
        char_select();
        break;
      case GAME:
        sound.StopBGM();
        status("GAME");
        map_start();
        if( state==GAME )
          game();
        break;
      default:
        state = EXIT;
        break;
    }
  }
  status("EXIT");
  exit_engine();
  return 0;
}
END_OF_MAIN();
