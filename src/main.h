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
 *  By ElvenProgrammer aka Eugenio Favalli (umperio@users.upagiro.net)
 */

#ifdef WIN32
  #pragma warning (disable:4312)
#endif

#ifndef _MAIN_H
#define _MAIN_H

#define CORE_VERSION "0.0.8"

#include <guichan.hpp>
#include <guichan/allegro.hpp>

#include "./configuration.h"
#include "./gui/login.h"
#include "./gui/gui.h"
#include "./gui/char_server.h"
#include "./gui/char_select.h"
#include "./gui/inventory.h"
#include "log.h"
#include "game.h"
#include "./net/protocol.h"
#include "./graphic/2xsai.h"
#include "../data/graphic/gfx_data.h"
#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#endif
#include <stdio.h>


#define EXIT        0
#define LOGIN       1
#define CHAR_SERVER 2
#define CHAR_SELECT 3
#define CHAR_NEW    4
#define CHAR_DEL    5
#define GAME        6

/* length definitions for several char[]s in order
 * to be able to use strncpy instead of strcpy for
 * security and stability reasons */

#define LEN_USERNAME 25
#define LEN_PASSWORD 25



typedef struct {
	int address;
	short port;
	char name[20];
	short online_users;
} SERVER_INFO;

typedef struct {
	int id;
	char name[24];
	short hp, max_hp, sp, max_sp, lv;
	int xp, gp, job_xp, job_lv;
	short statp, skill_point, hair_color, hair_style;
	char STR, AGI, VIT, INT, DEX, LUK;
	short weapon;
} PLAYER_INFO;

extern BITMAP *playerset;
extern DATAFILE *graphic, *emotions, *weaponset;
extern char username[25];
extern char password[25];
extern int map_address, char_ID;
extern short map_port;
extern char map_name[16];
extern int account_ID, session_ID1, session_ID2;
extern char sex, n_server, n_character;
extern SERVER_INFO *server_info;
extern PLAYER_INFO *char_info;
extern unsigned char state;
extern unsigned short x, y;
extern unsigned char direction;
//extern unsigned short job, hair, hair_color;
extern unsigned char stretch_mode;

#endif
