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

void Net::GameServer::Player::walk(int x, int y)
{
    MessageOut msg(PGMSG_WALK);
    msg.writeShort(x);
    msg.writeShort(y);
    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::pickUp(int x, int y)
{
    MessageOut msg(PGMSG_PICKUP);
    msg.writeShort(x);
    msg.writeShort(y);
    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::moveItem(int oldSlot, int newSlot, int amount)
{
    MessageOut msg(PGMSG_MOVE_ITEM);
    msg.writeByte(oldSlot);
    msg.writeByte(newSlot);
    msg.writeByte(amount);
    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::drop(int slot, int amount)
{
    MessageOut msg(PGMSG_DROP);
    msg.writeByte(slot);
    msg.writeByte(amount);
    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::equip(int slot)
{
    MessageOut msg(PGMSG_EQUIP);
    msg.writeByte(slot);
    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::unequip(int slot)
{
    MessageOut msg(PGMSG_UNEQUIP);
    msg.writeByte(slot);
    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::useItem(int slot)
{
    MessageOut msg(PGMSG_USE_ITEM);
    msg.writeByte(slot);
    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::attack(int direction)
{
    MessageOut msg(PGMSG_ATTACK);
    msg.writeByte(direction);
    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::changeAction(Being::Action action)
{
    MessageOut msg(PGMSG_ACTION_CHANGE);
    msg.writeByte(action);
    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::talkToNPC(int id, bool restart)
{
    MessageOut msg(restart ? PGMSG_NPC_TALK : PGMSG_NPC_TALK_NEXT);
    msg.writeShort(id);
    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::selectFromNPC(int id, int choice)
{
    MessageOut msg(PGMSG_NPC_SELECT);
    msg.writeShort(id);
    msg.writeByte(choice);
    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::requestTrade(int id)
{
    MessageOut msg(PGMSG_TRADE_REQUEST);
    msg.writeShort(id);
    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::acceptTrade(bool accept)
{
    MessageOut msg(accept ? PGMSG_TRADE_ACCEPT : PGMSG_TRADE_CANCEL);
    Net::GameServer::connection->send(msg);
}

void Net::GameServer::Player::tradeItem(int slot, int amount)
{
    MessageOut msg(PGMSG_TRADE_ADD_ITEM);
    msg.writeByte(slot);
    msg.writeByte(amount);
    Net::GameServer::connection->send(msg);
}
