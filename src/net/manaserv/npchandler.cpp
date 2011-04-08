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

#include "net/manaserv/npchandler.h"

#include "actorspritemanager.h"
#include "event.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/manaserv_protocol.h"

#include "utils/stringutils.h"

extern Net::NpcHandler *npcHandler;

namespace ManaServ {

extern Connection *gameServerConnection;

NpcHandler::NpcHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_NPC_CHOICE,
        GPMSG_NPC_POST,
        GPMSG_NPC_MESSAGE,
        GPMSG_NPC_ERROR,
        GPMSG_NPC_CLOSE,
        GPMSG_NPC_NUMBER,
        GPMSG_NPC_STRING,
        0
    };
    handledMessages = _messages;
    npcHandler = this;
}

void NpcHandler::handleMessage(Net::MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    if (!being || being->getType() != ActorSprite::NPC)
    {
        return;
    }

    int npcId = being->getId(), count = 0;
    Mana::Event *event = 0;

    switch (msg.getId())
    {
    case GPMSG_NPC_CHOICE:
        event = new Mana::Event(Mana::Event::Menu);
        event->setInt("id", npcId);
        while (msg.getUnreadLength())
        {
            count++;
            event->setString("choice" + toString(count), msg.readString());
        }
        event->setInt("choiceCount", count);
        event->trigger(CHANNEL_NPC);
        break;

    case GPMSG_NPC_NUMBER:
        event = new Mana::Event(Mana::Event::IntegerInput);
        event->setInt("id", npcId);
        event->setInt("min", msg.readInt32());
        event->setInt("max", msg.readInt32());
        event->setInt("default", msg.readInt32());
        event->trigger(CHANNEL_NPC);
        break;

    case GPMSG_NPC_STRING:
        event = new Mana::Event(Mana::Event::StringInput);
        event->setInt("id", npcId);
        event->trigger(CHANNEL_NPC);
        break;

    case GPMSG_NPC_POST:
        event = new Mana::Event(Mana::Event::Post);
        event->setInt("id", npcId);
        event->trigger(CHANNEL_NPC);
        break;

    case GPMSG_NPC_ERROR:
        event = new Mana::Event(Mana::Event::End);
        event->setInt("id", npcId);
        event->trigger(CHANNEL_NPC);
        break;

    case GPMSG_NPC_MESSAGE:
        event = new Mana::Event(Mana::Event::Message);
        event->setInt("id", npcId);
        event->setString("text", msg.readString(msg.getUnreadLength()));
        event->trigger(CHANNEL_NPC);
        delete event;

        event = new Mana::Event(Mana::Event::Next);
        event->setInt("id", npcId);
        event->trigger(CHANNEL_NPC);
        break;

    case GPMSG_NPC_CLOSE:
        event = new Mana::Event(Mana::Event::Close);
        event->setInt("id", npcId);
        event->trigger(CHANNEL_NPC);
        break;
    }

    delete event;
}

void NpcHandler::startShopping(int beingId)
{
    // TODO
}

void NpcHandler::buy(int beingId)
{
    // TODO
}

void NpcHandler::sell(int beingId)
{
    // TODO
}

void NpcHandler::buyItem(int beingId, int itemId, int amount)
{
    MessageOut msg(PGMSG_NPC_BUYSELL);
    msg.writeInt16(itemId);
    msg.writeInt16(amount);
    gameServerConnection->send(msg);
}

void NpcHandler::sellItem(int beingId, int itemId, int amount)
{
    MessageOut msg(PGMSG_NPC_BUYSELL);
    msg.writeInt16(itemId);
    msg.writeInt16(amount);
    gameServerConnection->send(msg);
}

void NpcHandler::endShopping(int beingId)
{
    // TODO
}

void NpcHandler::talk(int npcId)
{
    MessageOut msg(PGMSG_NPC_TALK);
    msg.writeInt16(npcId);
    gameServerConnection->send(msg);

    Mana::Event event(Mana::Event::TalkSent);
    event.setInt("npcId", npcId);
    event.trigger(CHANNEL_NPC);
}

void NpcHandler::nextDialog(int npcId)
{
    MessageOut msg(PGMSG_NPC_TALK_NEXT);
    msg.writeInt16(npcId);
    gameServerConnection->send(msg);

    Mana::Event event(Mana::Event::NextSent);
    event.setInt("npcId", npcId);
    event.trigger(CHANNEL_NPC);
}

void NpcHandler::closeDialog(int npcId)
{
    MessageOut msg(PGMSG_NPC_TALK_NEXT);
    msg.writeInt16(npcId);
    gameServerConnection->send(msg);

    Mana::Event event(Mana::Event::CloseSent);
    event.setInt("npcId", npcId);
    event.trigger(CHANNEL_NPC);
}

void NpcHandler::menuSelect(int npcId, int choice)
{
    MessageOut msg(PGMSG_NPC_SELECT);
    msg.writeInt16(npcId);
    msg.writeInt8(choice);
    gameServerConnection->send(msg);

    Mana::Event event(Mana::Event::MenuSent);
    event.setInt("npcId", npcId);
    event.setInt("choice", choice);
    event.trigger(CHANNEL_NPC);
}

void NpcHandler::integerInput(int npcId, int value)
{
    MessageOut msg(PGMSG_NPC_NUMBER);
    msg.writeInt16(npcId);
    msg.writeInt32(value);
    gameServerConnection->send(msg);

    Mana::Event event(Mana::Event::IntegerInputSent);
    event.setInt("npcId", npcId);
    event.setInt("value", value);
    event.trigger(CHANNEL_NPC);
}

void NpcHandler::stringInput(int npcId, const std::string &value)
{
    MessageOut msg(PGMSG_NPC_STRING);
    msg.writeInt16(npcId);
    msg.writeString(value);
    gameServerConnection->send(msg);

    Mana::Event event(Mana::Event::StringInputSent);
    event.setInt("npcId", npcId);
    event.setString("value", value);
    event.trigger(CHANNEL_NPC);
}

void NpcHandler::sendLetter(int npcId, const std::string &recipient,
                const std::string &text)
{
    MessageOut msg(PGMSG_NPC_POST_SEND);
    msg.writeString(recipient);
    msg.writeString(text);
    gameServerConnection->send(msg);

    Mana::Event event(Mana::Event::SendLetterSent);
    event.setInt("npcId", npcId);
    event.setString("recipient", recipient);
    event.setString("text", text);
    event.trigger(CHANNEL_NPC);
}

} // namespace ManaServ
