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

#ifndef _TMW_PROTOCOL_H
#define _TMW_PROTOCOL_H

/** Packet length by id */
short get_length(short id);

/** Decodes x coord */
unsigned short get_x(const char *data);

/** Decodes y coord */
unsigned short get_y(const char *data);

/** Decodes direction */
unsigned char get_direction(const char *data);

/** Decodes src x coord */
unsigned short get_src_x(const char *data);

/** Decodes src y coord */
unsigned short get_src_y(const char *data);

/** Decodes src direction */
unsigned char get_src_direction(char data);

/** Decodes dest x coord */
unsigned short get_dest_x(const char *data);

/** Decodes dest y coord */
unsigned short get_dest_y(const char *data);

/** Decodes dest direction */
unsigned char get_dest_direction(char data);

/** Encodes coords and direction in 3 bytes data */
void set_coordinates(char *data, unsigned short x, unsigned short y, unsigned char direction);

/** Initialize connection with map server */
void map_start();

/** Requests to walk */
void walk(unsigned short x, unsigned short y, unsigned char direction);

/** Request to speak */
void speak(char *speech);

/** Request to attack */
void attack(unsigned short x, unsigned short y, unsigned char direction);

/** Request action */
void action(char type, int id);

#endif
