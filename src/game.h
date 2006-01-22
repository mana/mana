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

#ifndef _TMW_GAME_
#define _TMW_GAME_

#include <iosfwd>

#define SPEECH_TIME 80
#define SPEECH_MAX_TIME 100

class Network;
class NPC;

extern std::string map_path;
extern int fps;
extern volatile int tick_time;
extern int server_tick;

enum {
    JOY_UP,
    JOY_DOWN,
    JOY_LEFT,
    JOY_RIGHT,
    JOY_BTN0,
    JOY_BTN1,
    JOY_BTN2,
    JOY_BTN3,
    JOY_BTN4,
    JOY_BTN5,
    JOY_BTN6,
    JOY_BTN7,
    JOY_BTN8,
    JOY_BTN9,
    JOY_BTN10,
    JOY_BTN11
};

/**
 * Main game loop
 */
void game(Network*);

/**
 * Check user input
 */
void do_input(Network*);

/**
 * Parse data received from map server into input buffer
 */
void do_parse(Network*);

/**
 * Clean the engine
 */
void do_exit(Network*);

/**
 * Returns elapsed time. (Warning: very unsafe function, it supposes the delay
 * is always < 10 seconds)
 */
int get_elapsed_time(int start_time);

#endif
