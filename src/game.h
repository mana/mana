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

#ifndef _TMW_GAME_H
#define _TMW_GAME_H

#include "main.h"
#include "./gui/gui.h"
#include "./gui/skill.h"
#include <stdio.h>

#define SPEECH_TIME 80
#define SPEECH_MAX_TIME 100

// Action defines
#define STAND       0
#define WALK        1
#define ATTACK      5
#define BOW_ATTACK  9
#define SIT        13
#define HIT        14
#define DEAD       15
#define MONSTER_ATTACK 5
#define MONSTER_DEAD 9

#define LOCK 254
#define IDLE 255

#define DIR_NONE -1
#define SOUTH 0
#define SW    1
#define WEST  2
#define NW    3
#define NORTH 4
#define NE    5
#define EAST  6
#define SE    7

extern char map_path[480];
extern int fps, frame, current_npc;
extern volatile int tick_time;
extern int server_tick;
extern bool displayPathToMouse;
extern int startX, startY;
extern int autoTarget;

/**
 * Main game loop
 */
void game();

/**
 * Initialize game engine
 */
void do_init();

/**
 * Check user input
 */
void do_input();

/**
 * Parse data received from map server into input buffer
 */
void do_parse();

/**
 * Clean the engine
 */
void do_exit();

/**
 * Calculate packet length
 */
int get_packet_length(short);

/**
 * Returns elapsed time. (Warning: very unsafe function, it supposes the delay
 * is always < 10 seconds)
 */
int get_elapsed_time(int start_time);

#endif
