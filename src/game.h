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

#ifdef WIN32
  #pragma warning (disable:4312)
#endif

#ifndef _GAME_H
#define _GAME_H

#include <allegro.h>
#include <stdio.h>

#include "main.h"
#include "./gui/gui.h"
#include "./gui/skill.h"
#include "./gui/stats.h"
#include "./graphic/2xsai.h"

#define SPEECH_TIME 40
#define EMOTION_TIME 40
#define SPEECH_MAX_TIME 100

#define MAKECOL_WHITE makecol(255,255,255)
#define MAKECOL_BLACK makecol(0,0,0)

#define GET_WALKING_SPEECH_XCOORDS ((get_x(node->coordinates)-map_x+13)*16-25-(alfont_text_length(gui_font, node->speech)/2)-player_x-coeff_x*(16-4*node->frame))
#define GET_WALKING_SPEECH_YCOORDS ((get_y(node->coordinates)-map_y+6)*16-55-player_y-coeff_y*(16-4*node->frame))

#define GET_STANDING_SPEECH_XCOODRS ((get_x(node->coordinates)-map_x+13)*16-25-(alfont_text_length(gui_font, node->speech)/2)-player_x)
#define GET_STANDING_SPEECH_YCOORDS ((get_y(node->coordinates)-map_y+6)*16-55-player_y)

#define PLAYERSETS 4
#define SIT   17
#define STAND 0
#define WALK 1
#define MONSTER_ATTACK 5
#define MONSTER_DEAD 9
#define ATTACK 7
#define LOCK 254
#define IDLE 255

#define SOUTH 0
#define SW    1
#define WEST  2
#define NW    3
#define NORTH 4
#define NE    5
#define EAST  6
#define SE    7

extern char map_path[480];
extern DIALOG skill_dialog[];
extern DIALOG stats_dialog[];
extern int fps, frame, current_npc;
extern char walk_status;
extern unsigned short src_x, src_y, x, y;
extern volatile int tick_time;

void game();
void do_init();
void do_input();
void do_parse();
void do_graphic();
void do_logic();
void do_exit();
int  get_packet_length(short);

char get_x_offset(char, char);
char get_y_offset(char, char);

short get_elapsed_time(short start_time);

#endif
