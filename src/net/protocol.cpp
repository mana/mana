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

#include "../main.h"
#include "../being.h"
#include "../game.h"
#include "protocol.h"
#include "network.h"

#ifdef WIN32
#include <windows.h>
#else
#include "win2linux.h"
#endif


short packet_lengths[] = {
   10,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
// #0x0040
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0, 55, 17,  3, 37, 46, -1, 23, -1,  3,108,  3,  2,
    3, 28, 19, 11,  3, -1,  9,  5, 54, 53, 58, 60, 41,  2,  6,  6,
// #0x0080
    7,  3,  2,  2,  2,  5, 16, 12, 10,  7, 29, 23, -1, -1, -1,  0,
    7, 22, 28,  2,  6, 30, -1, -1,  3, -1, -1,  5,  9, 17, 17,  6,
   23,  6,  6, -1, -1, -1, -1,  8,  7,  6,  7,  4,  7,  0, -1,  6,
    8,  8,  3,  3, -1,  6,  6, -1,  7,  6,  2,  5,  6, 44,  5,  3,
// #0x00C0
    7,  2,  6,  8,  6,  7, -1, -1, -1, -1,  3,  3,  6,  6,  2, 27,
    3,  4,  4,  2, -1, -1,  3, -1,  6, 14,  3, -1, 28, 29, -1, -1,
   30, 30, 26,  2,  6, 26,  3,  3,  8, 19,  5,  2,  3,  2,  2,  2,
    3,  2,  6,  8, 21,  8,  8,  2,  2, 26,  3, -1,  6, 27, 30, 10,
// #0x0100
    2,  6,  6, 30, 79, 31, 10, 10, -1, -1,  4,  6,  6,  2, 11, -1,
   10, 39,  4, 10, 31, 35, 10, 18,  2, 13, 15, 20, 68,  2,  3, 16,
    6, 14, -1, -1, 21,  8,  8,  8,  8,  8,  2,  2,  3,  4,  2, -1,
    6, 86,  6, -1, -1,  7, -1,  6,  3, 16,  4,  4,  4,  6, 24, 26,
// #0x0140
   22, 14,  6, 10, 23, 19,  6, 39,  8,  9,  6, 27, -1,  2,  6,  6,
  110,  6, -1, -1, -1, -1, -1,  6, -1, 54, 66, 54, 90, 42,  6, 42,
   -1, -1, -1, -1, -1, 30, -1,  3, 14,  3, 30, 10, 43, 14,186,182,
   14, 30, 10,  3, -1,  6,106, -1,  4,  5,  4, -1,  6,  7, -1, -1,
// #0x0180
    6,  3,106, 10, 10, 34,  0,  6,  8,  4,  4,  4, 29, -1, 10,  6,
   90, 86, 24,  6, 30,102,  9,  4,  8,  4, 14, 10,  4,  6,  2,  6,
    3,  3, 35,  5, 11, 26, -1,  4,  4,  6, 10, 12,  6, -1,  4,  4,
   11,  7, -1, 67, 12, 18,114,  6,  3,  6, 26, 26, 26, 26,  2,  3,
// #0x01C0
    2, 14, 10, -1, 22, 22,  4,  2, 13, 97,  0,  9,  9, 29,  6, 28,
    8, 14, 10, 35,  6,  8,  4, 11, 54, 53, 60,  2, -1, 47, 33,  6,
   30,  8, 34, 14,  2,  6, 26,  2, 28, 81,  6, 10, 26,  2, -1, -1,
   -1, -1, 20, 10, 32,  9, 34, 14,  2,  6, 48, 56, -1,  4,  5, 10,
// #0x200
   26,  0,  0,  0, 18,  0,  0,  0,  0,  0,  0, 19,
};

short get_length(short id) {
    return packet_lengths[id];
}

unsigned short get_dest_x(const char *data) {
    short temp;
    temp = MAKEWORD(data[3], data[2] & 0x000f);
    temp >>= 2;
    return temp;
}

unsigned short get_dest_y(const char *data) {
    return MAKEWORD(data[4], data[3] & 0x0003);
}

unsigned short get_src_x(const char *data) {
    short temp;
    temp = MAKEWORD(data[1], data[0]);
    temp >>= 6;
    return temp;
}

unsigned short get_src_y(const char *data) {
    short temp;
    temp = MAKEWORD(data[2], data[1] & 0x003f);
    temp >>= 4;
    return temp;
}

unsigned char get_src_direction(char data) {
    data >>= 4;
    return data;
}

unsigned char get_dest_direction(char data) {
    return data & 0x000f;
}

unsigned short get_x(const char *data) {
    short temp;
    temp = MAKEWORD(data[1] & 0x00c0, data[0] & 0x00ff);
    temp >>= 6;
    return temp;
}

unsigned short get_y(const char *data) {
    short temp;
    if (!data) throw "Corrupted data";
    temp = MAKEWORD(data[2] & 0x00f0, data[1] & 0x003f);
    temp >>= 4;
    return temp;
}

unsigned char get_direction(const char *data) {
    return data[2] & 0x000f;
}

void set_coordinates(char *data, unsigned short x, unsigned short y,
        unsigned char direction)
{
    short temp;
    temp = x;
    temp <<= 6;
    data[0] = 0;
    data[1] = 1;
    data[2] = 2;
    data[0] = HIBYTE(temp);
    data[1] = LOBYTE(temp);
    temp = y;
    temp <<= 4;
    data[1] |= HIBYTE(temp);
    data[2] = LOBYTE(temp);
    data[2] |= direction;
}

void map_start()
{
    // Connect to map server
    if (open_session(iptostring(map_address), map_port) == SOCKET_ERROR)
    {
        logger->log("Warning: Unable to connect to map server");
        throw "Unable to connect to map server";
        return;
    }

    // Send login infos
    WFIFOW(0) = net_w_value(0x0072);
    WFIFOL(2) = net_l_value(account_ID);
    WFIFOL(6) = net_l_value(char_ID);
    WFIFOL(10) = net_l_value(session_ID1);
    WFIFOL(14) = net_l_value(session_ID2);
    WFIFOB(18) = net_b_value(sex);
    WFIFOSET(19);

    while ((in_size < 4)|| (out_size > 0)) flush();
    RFIFOSKIP(4);

    while (in_size < 2) flush();

    if (RFIFOW(0) == SMSG_LOGIN_SUCCESS) {
        while (in_size < 11) flush();
        startX = get_x(RFIFOP(6));
        startY = get_y(RFIFOP(6));
        int direction = get_direction(RFIFOP(6));
        logger->log("Protocol: Player start position: (%d, %d), Direction: %d",
                startX, startY, direction);
        RFIFOSKIP(11);
    } else if (0x0081) {
        logger->log("Warning: Map server D/C");
    } else {
        logger->error("Unknown packet: map_start");
    }

    // Send "map loaded"
    WFIFOW(0) = net_w_value(0x007d);
    WFIFOSET(2);
    while (out_size > 0) flush();
}

void walk(unsigned short x, unsigned short y, unsigned char direction)
{
    char temp[3];
    set_coordinates(temp, x, y, direction);
    WFIFOW(0) = net_w_value(0x0085);
    memcpy(WFIFOP(2), temp, 3);
    WFIFOSET(5);
}

void speak(char *speech)
{
    int len = (int)strlen(speech);
    WFIFOW(0) = net_w_value(0x008c);
    WFIFOW(2) = net_w_value(len + 4);
    memcpy(WFIFOP(4), speech, len);
    WFIFOSET(len + 4);
}

void action(char type, int id)
{
    WFIFOW(0) = net_w_value(0x0089);
    WFIFOL(2) = net_l_value(id);
    WFIFOB(6) = net_b_value(type);
    WFIFOSET(7);
}

void attack(unsigned short x, unsigned short y, unsigned char direction)
{
    int monster_id = 0;

    if (direction == SOUTH) {
        monster_id = findMonster(x, y + 1);
        if (monster_id != 0)
            action(0, monster_id);
    } else if(direction == WEST) {
        monster_id = findMonster(x - 1, y);
        if (monster_id != 0)
            action(0, monster_id);
    } else if(direction == NORTH) {
        monster_id = findMonster(x, y - 1);
        if (monster_id != 0)
            action(0, monster_id);
    } else if(direction==EAST) {
        monster_id = findMonster(x + 1, y);
        if (monster_id != 0)
            action(0, monster_id);
    }
    // implement charging attacks here
    char_info->lastAttackTime = 0;
}
