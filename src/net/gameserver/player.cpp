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

#include "player.h"

#include "internal.h"

#include "../connection.h"
#include "../messageout.h"
#include "../protocol.h"

void Net::GameServer::Player::say(const std::string &text)
{
    MessageOut msg(PGMSG_SAY);

    msg.writeString(text);

    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::walk(short x, short y)
{
    MessageOut msg(PGMSG_WALK);

    msg.writeShort(x);
    msg.writeShort(y);

    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::useItem(int itemId)
{
    MessageOut msg(PGMSG_USE_ITEM);

    msg.writeLong(itemId);

    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::equip(int itemId, char slot)
{
    MessageOut msg(PGMSG_EQUIP);

    msg.writeLong(itemId);
    msg.writeByte(slot);

    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::attack(unsigned char direction)
{
    MessageOut msg(PGMSG_ATTACK);
    msg.writeByte(direction);
    Net::GameServer::connection->send(msg);
}
