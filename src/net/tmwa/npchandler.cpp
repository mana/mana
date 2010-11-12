/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "net/tmwa/npchandler.h"

#include "actorspritemanager.h"
#include "event.h"
#include "localplayer.h"

#include "net/messagein.h"
#include "net/messageout.h"
#include "net/net.h"
#include "net/npchandler.h"

#include "net/tmwa/protocol.h"

#include "utils/stringutils.h"

#include <SDL_types.h>

extern Net::NpcHandler *npcHandler;

static void parseMenu(Mana::Event *event, const std::string &options)
{
    std::istringstream iss(options);

    int count = 0;
    std::string tmp;
    while (getline(iss, tmp, ':'))
    {
        count++;
        event->setString("choice" + toString(count), tmp);
    }

    event->setInt("choiceCount", count);
}

namespace TmwAthena {

NpcHandler::NpcHandler()
{
    static const Uint16 _messages[] = {
        SMSG_NPC_CHOICE,
        SMSG_NPC_MESSAGE,
        SMSG_NPC_NEXT,
        SMSG_NPC_CLOSE,
        SMSG_NPC_INT_INPUT,
        SMSG_NPC_STR_INPUT,
        0
    };
    handledMessages = _messages;
    npcHandler = this;

    listen(CHANNEL_NPC);
}

void NpcHandler::handleMessage(Net::MessageIn &msg)
{
    if (msg.getId() == SMSG_NPC_CHOICE || msg.getId() == SMSG_NPC_MESSAGE)
    {
        msg.readInt16();  // length
    }

    int npcId = msg.readInt32();
    Mana::Event *event = 0;

    switch (msg.getId())
    {
    case SMSG_NPC_CHOICE:
        event = new Mana::Event(EVENT_MENU);
        event->setInt("id", npcId);
        parseMenu(event, msg.readString(msg.getLength() - 8));
        event->trigger(CHANNEL_NPC);
        break;

    case SMSG_NPC_MESSAGE:
        event = new Mana::Event(EVENT_MESSAGE);
        event->setInt("id", npcId);
        event->setString("text", msg.readString(msg.getLength() - 8));
        event->trigger(CHANNEL_NPC);
        break;

     case SMSG_NPC_CLOSE:
        // Show the close button
        event = new Mana::Event(EVENT_CLOSE);
        event->setInt("id", npcId);
        event->trigger(CHANNEL_NPC);
        break;

    case SMSG_NPC_NEXT:
        // Show the next button
        event = new Mana::Event(EVENT_NEXT);
        event->setInt("id", npcId);
        event->trigger(CHANNEL_NPC);
        break;

    case SMSG_NPC_INT_INPUT:
        // Request for an integer
        event = new Mana::Event(EVENT_INTEGERINPUT);
        event->setInt("id", npcId);
        event->trigger(CHANNEL_NPC);
        break;

    case SMSG_NPC_STR_INPUT:
        // Request for a string
        event = new Mana::Event(EVENT_STRINGINPUT);
        event->setInt("id", npcId);
        event->trigger(CHANNEL_NPC);
        break;
    }

    delete event;

    if (player_node->getCurrentAction() != Being::SIT)
        player_node->setAction(Being::STAND);
}

void NpcHandler::event(Channels channel, const Mana::Event &event)
{
    if (channel == CHANNEL_NPC)
    {
        if (event.getName() == EVENT_DOTALK)
        {
            MessageOut outMsg(CMSG_NPC_TALK);
            outMsg.writeInt32(event.getInt("npcId"));
            outMsg.writeInt8(0); // Unused
        }
        else if (event.getName() == EVENT_DONEXT)
        {
            MessageOut outMsg(CMSG_NPC_NEXT_REQUEST);
            outMsg.writeInt32(event.getInt("npcId"));
        }
        else if (event.getName() == EVENT_DOCLOSE)
        {
            MessageOut outMsg(CMSG_NPC_CLOSE);
            outMsg.writeInt32(event.getInt("npcId"));
        }
        else if (event.getName() == EVENT_DOMENU)
        {
            MessageOut outMsg(CMSG_NPC_LIST_CHOICE);
            outMsg.writeInt32(event.getInt("npcId"));
            outMsg.writeInt8(event.getInt("choice"));
        }
        else if (event.getName() == EVENT_DOINTEGERINPUT)
        {
            MessageOut outMsg(CMSG_NPC_INT_RESPONSE);
            outMsg.writeInt32(event.getInt("npcId"));
            outMsg.writeInt32(event.getInt("value"));
        }
        else if (event.getName() == EVENT_DOSTRINGINPUT)
        {
            const std::string &value = event.getString("value");
            MessageOut outMsg(CMSG_NPC_STR_RESPONSE);
            outMsg.writeInt16(value.length() + 9);
            outMsg.writeInt32(event.getInt("npcId"));
            outMsg.writeString(value, value.length());
            outMsg.writeInt8(0); // Prevent problems with string reading
        }
    }
}

void NpcHandler::startShopping(int beingId)
{
    // TODO
}

void NpcHandler::buy(int beingId)
{
    MessageOut outMsg(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(beingId);
    outMsg.writeInt8(0); // Buy
}

void NpcHandler::sell(int beingId)
{
    MessageOut outMsg(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(beingId);
    outMsg.writeInt8(1); // Sell
}

void NpcHandler::buyItem(int beingId, int itemId, int amount)
{
    MessageOut outMsg(CMSG_NPC_BUY_REQUEST);
    outMsg.writeInt16(8); // One item (length of packet)
    outMsg.writeInt16(amount);
    outMsg.writeInt16(itemId);
}

void NpcHandler::sellItem(int beingId, int itemId, int amount)
{
    MessageOut outMsg(CMSG_NPC_SELL_REQUEST);
    outMsg.writeInt16(8); // One item (length of packet)
    outMsg.writeInt16(itemId + INVENTORY_OFFSET);
    outMsg.writeInt16(amount);
}

void NpcHandler::endShopping(int beingId)
{
    // TODO
}

} // namespace TmwAthena
