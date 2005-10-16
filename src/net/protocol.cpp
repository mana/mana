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

#include "protocol.h"

#include "messagein.h"
#include "messageout.h"
#include "network.h"

#include "../being.h"
#include "../game.h"
#include "../log.h"
#include "../main.h"
#include "../playerinfo.h"
#include "../sound.h"

#define LOBYTE(w)  ((unsigned char)(w))
#define HIBYTE(w)  ((unsigned char)(((unsigned short)(w)) >> 8))

unsigned char get_src_direction(char data)
{
    data >>= 4;
    return data;
}

unsigned char get_dest_direction(char data)
{
    return data & 0x000f;
}

void set_coordinates(char *data,
                     unsigned short x,
                     unsigned short y,
                     unsigned char direction)
{
    short temp;
    temp = x;
    temp <<= 6;
    data[0] = 0;
    data[1] = 1;
    data[2] = 2;
    data[0] = HIBYTE(temp);
    data[1] = (unsigned char)(temp);
    temp = y;
    temp <<= 4;
    data[1] |= HIBYTE(temp);
    data[2] = LOBYTE(temp);
    data[2] |= direction;
}

void walk(unsigned short x, unsigned short y, unsigned char direction)
{
    char temp[3];
    MessageOut outMsg;
    set_coordinates(temp, x, y, direction);
    outMsg.writeShort(0x0085);
    outMsg.writeString(temp, 3);
}

void action(char type, int id)
{
    MessageOut outMsg;
    outMsg.writeShort(0x0089);
    outMsg.writeLong(id);
    outMsg.writeByte(type);
}

Being* attack(unsigned short x, unsigned short y, unsigned char direction)
{
    Being *target = NULL;

    switch (direction)
    {
        case Being::SOUTH:
            target = findNode(x, y + 1, Being::MONSTER);
            if (!target) target = findNode(x, y + 1, Being::PLAYER);
            break;

        case Being::WEST:
            target = findNode(x - 1, y, Being::MONSTER);
            if (!target) target = findNode(x - 1, y, Being::PLAYER);
            break;

        case Being::NORTH:
            target = findNode(x, y - 1, Being::MONSTER);
            if (!target) target = findNode(x, y - 1, Being::PLAYER);
            break;

        case Being::EAST:
            target = findNode(x + 1, y, Being::MONSTER);
            if (!target) target = findNode(x + 1, y, Being::PLAYER);
            break;
    }

    if (target) {
        attack(target);
    }

    return target;
}

void attack(Being *target)
{
    int dist_x = target->x - player_node->x;
    int dist_y = target->y - player_node->y;

    if (abs(dist_y) >= abs(dist_x))
    {
        if (dist_y > 0)
            player_node->direction = Being::SOUTH;
        else
            player_node->direction = Being::NORTH;
    }
    else
    {
        if (dist_x > 0)
            player_node->direction = Being::EAST;
        else
            player_node->direction = Being::WEST;
    }

    // Implement charging attacks here
    player_info->lastAttackTime = 0;

    player_node->action = Being::ATTACK;
    action(0, target->getId());
    player_node->walk_time = tick_time;
    if (player_node->getWeapon() == 2)
        sound.playSfx("sfx/bow_shoot_1.ogg");
    else
        sound.playSfx("sfx/fist-swish.ogg");
}
