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
  
#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <allegro.h>
#ifdef WIN32
#include <winalleg.h>
#include <windows.h>
#else
#include "win2linux.h"
#endif

#include "../main.h"
#include "../being.h"

short get_length(short id);
unsigned short get_x(char *data);
unsigned short get_y(char *data);
unsigned char get_direction(char *data);
unsigned short get_src_x(char *data);
unsigned short get_src_y(char *data);
unsigned char get_src_direction(char data);
unsigned short get_dest_x(char *data);
unsigned short get_dest_y(char *data);
unsigned char get_dest_direction(char data);
void set_coordinates(char *data, unsigned short x, unsigned short y, unsigned char direction);
void map_start();
void walk(unsigned short x, unsigned short y, unsigned char direction);
void speak(char *speech);
void attack(unsigned short x, unsigned short y, unsigned char direction);
void action(char type, int id);

#endif
