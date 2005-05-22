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

// Packets from server to client
#define SMSG_LOGIN_SUCCESS        0x0073 /**< Logged in, starting location */
#define SMSG_REMOVE_BEING         0x0080 /**< Died, logged out, teleport ... */
#define SMSG_MOVE_BEING           0x007b /**< A nearby monster moves */
#define SMSG_PLAYER_UPDATE_1      0x01d8
#define SMSG_PLAYER_UPDATE_2      0x01d9
#define SMSG_MOVE_PLAYER_BEING    0x01da /**< A nearby player moves */
#define SMSG_CHANGE_BEING_LOOKS   0x00c3
#define SMSG_BEING_CHAT           0x008d /**< A being talks */
#define SMSG_MY_BEING_CHAT        0x008e /**< My being talks */
#define SMSG_GM_CHAT              0x009a /**< GM announce */
#define SMSG_WALK_RESPONSE        0x0087


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

/** Request to attack */
void attack(Being *target);

/** Request action */
void action(char type, int id);

#endif
