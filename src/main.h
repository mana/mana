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

#ifndef _TMW_MAIN_H
#define _TMW_MAIN_H

#include "gui/gui.h"
#include "gui/login.h"
#include "gui/skill.h"
#include "graphic/spriteset.h"
#include "resources/image.h"
#include "resources/itemmanager.h"
#include "configuration.h"
#include "game.h"
#include "log.h"
#include "map.h"
#include "sound.h"
#include <stdio.h>
#include <memory>

#ifndef TMW_DATADIR
#define TMW_DATADIR ""
#endif

#ifdef ERROR
#undef ERROR
#endif

#define EXIT        0
#define LOGIN       1
#define CHAR_SERVER 2
#define CHAR_SELECT 3
#define CHAR_NEW    4
#define CHAR_DEL    5
#define GAME        6
#define ERROR       7

/* length definitions for several char[]s in order
 * to be able to use strncpy instead of strcpy for
 * security and stability reasons
 */
#define LEN_USERNAME 25
#define LEN_PASSWORD 25

#define N_SKILLS 100 // skill count constant value
#define XP_CONSTANT 1.2 // the exponent which determines skill exp curve

typedef struct {
    int address;
    short port;
    char name[20];
    short online_users;
} SERVER_INFO;

typedef struct {
    int id;
    float lastAttackTime; // used to synchronize the charge dialog
    char name[24];
    short hp, max_hp, sp, max_sp, lv;
    short statsPointsToAttribute;
    int xp, xpForNextLevel, gp, job_xp, jobXpForNextLevel, job_lv;
    short statp, skill_point, hair_color, hair_style;
    char STR, AGI, VIT, INT, DEX, LUK;
    char STRUp, AGIUp, VITUp, INTUp, DEXUp, LUKUp;
    short weapon;  
    // skill list declaration
    std::vector<SKILL> m_Skill; // array of N_SKILLS skills
    // gets the requested skills level from char_info
    int GetSkill(int n_ID, int n_XP=2, int n_base = false); // implemented in the body (main.cpp)
} PLAYER_INFO;


extern Image *login_wallpaper;
extern Spriteset *hairset, *playerset;
extern Graphics *graphics;
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
extern Configuration config;
extern Sound sound;
extern Map *tiledMap;
extern Logger *logger;
extern int screenW, screenH, bitDepth, displayFlags;
extern bool useOpenGL;
extern ItemManager *itemDb;
extern char *homeDir;
extern volatile int framesToDraw;

#endif
