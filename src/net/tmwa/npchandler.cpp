/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

extern Net::NpcHandler *npcHandler;

static void parseMenu(Event *event, const std::string &options)
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
    static const uint16_t _messages[] = {
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
}

void NpcHandler::handleMessage(Net::MessageIn &msg)
{
    if (msg.getId() == SMSG_NPC_CHOICE || msg.getId() == SMSG_NPC_MESSAGE)
    {
        msg.readInt16();  // length
    }

    int npcId = msg.readInt32();
    Event *event = 0;

    switch (msg.getId())
    {
    case SMSG_NPC_CHOICE:
        event = new Event(Event::Menu);
        event->setInt("id", npcId);
        parseMenu(event, msg.readString(msg.getLength() - 8));
        event->trigger(Event::NpcChannel);
        break;

    case SMSG_NPC_MESSAGE:
        event = new Event(Event::Message);
        event->setInt("id", npcId);
        event->setString("text", msg.readString(msg.getLength() - 8));
        event->trigger(Event::NpcChannel);
        break;

     case SMSG_NPC_CLOSE:
        // Show the close button
        event = new Event(Event::Close);
        event->setInt("id", npcId);
        event->trigger(Event::NpcChannel);
        break;

    case SMSG_NPC_NEXT:
        // Show the next button
        event = new Event(Event::Next);
        event->setInt("id", npcId);
        event->trigger(Event::NpcChannel);
        break;

    case SMSG_NPC_INT_INPUT:
        // Request for an integer
        event = new Event(Event::IntegerInput);
        event->setInt("id", npcId);
        event->trigger(Event::NpcChannel);
        break;

    case SMSG_NPC_STR_INPUT:
        // Request for a string
        event = new Event(Event::StringInput);
        event->setInt("id", npcId);
        event->trigger(Event::NpcChannel);
        break;
    }

    delete event;

    if (local_player->getCurrentAction() != Being::SIT)
        local_player->setAction(Being::STAND);
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

void NpcHandler::talk(int npcId)
{
    MessageOut outMsg(CMSG_NPC_TALK);
    outMsg.writeInt32(npcId);
    outMsg.writeInt8(0); // Unused

    Event event(Event::TalkSent);
    event.setInt("npcId", npcId);
    event.trigger(Event::NpcChannel);
}

void NpcHandler::nextDialog(int npcId)
{
    MessageOut outMsg(CMSG_NPC_NEXT_REQUEST);
    outMsg.writeInt32(npcId);

    Event event(Event::NextSent);
    event.setInt("npcId", npcId);
    event.trigger(Event::NpcChannel);
}

void NpcHandler::closeDialog(int npcId)
{
    MessageOut outMsg(CMSG_NPC_CLOSE);
    outMsg.writeInt32(npcId);

    Event event(Event::CloseSent);
    event.setInt("npcId", npcId);
    event.trigger(Event::NpcChannel);
}

void NpcHandler::menuSelect(int npcId, int choice)
{
    MessageOut outMsg(CMSG_NPC_LIST_CHOICE);
    outMsg.writeInt32(npcId);
    outMsg.writeInt8(choice);

    Event event(Event::MenuSent);
    event.setInt("npcId", npcId);
    event.setInt("choice", choice);
    event.trigger(Event::NpcChannel);
}

void NpcHandler::integerInput(int npcId, int value)
{
    MessageOut outMsg(CMSG_NPC_INT_RESPONSE);
    outMsg.writeInt32(npcId);
    outMsg.writeInt32(value);

    Event event(Event::IntegerInputSent);
    event.setInt("npcId", npcId);
    event.setInt("value", value);
    event.trigger(Event::NpcChannel);
}

void NpcHandler::stringInput(int npcId, const std::string &value)
{
    MessageOut outMsg(CMSG_NPC_STR_RESPONSE);
    outMsg.writeInt16(value.length() + 9);
    outMsg.writeInt32(npcId);
    outMsg.writeString(value, value.length());
    outMsg.writeInt8(0); // Prevent problems with string reading

    Event event(Event::StringInputSent);
    event.setInt("npcId", npcId);
    event.setString("value", value);
    event.trigger(Event::NpcChannel);
}

void NpcHandler::sendLetter(int npcId, const std::string &recipient,
            const std::string &text)
{
    //NOTE: eA doesn't have letters
}

} // namespace TmwAthena
