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
#include "eventmanager.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/protocol.h"

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

    Mana::EventManager::bind(this, "NPC");
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
        event = new Mana::Event("Menu");
        event->setInt("id", npcId);
        while (msg.getUnreadLength())
        {
            count++;
            event->setString("choice" + toString(count), msg.readString());
        }
        event->setInt("choiceCount", count);
        Mana::EventManager::trigger("NPC", *event);
        break;

    case GPMSG_NPC_NUMBER:
        event = new Mana::Event("IntegerInput");
        event->setInt("id", npcId);
        event->setInt("min", msg.readInt32());
        event->setInt("max", msg.readInt32());
        event->setInt("default", msg.readInt32());
        Mana::EventManager::trigger("NPC", *event);
        break;

    case GPMSG_NPC_STRING:
        event = new Mana::Event("StringInput");
        event->setInt("id", npcId);
        Mana::EventManager::trigger("NPC", *event);
        break;

    case GPMSG_NPC_POST:
        event = new Mana::Event("Post");
        event->setInt("id", npcId);
        Mana::EventManager::trigger("NPC", *event);
        break;

    case GPMSG_NPC_ERROR:
        event = new Mana::Event("End");
        event->setInt("id", npcId);
        Mana::EventManager::trigger("NPC", *event);
        break;

    case GPMSG_NPC_MESSAGE:
        event = new Mana::Event("Message");
        event->setInt("id", npcId);
        event->setString("text", msg.readString(msg.getUnreadLength()));
        Mana::EventManager::trigger("NPC", *event);
        delete event;

        event = new Mana::Event("Next");
        event->setInt("id", npcId);
        Mana::EventManager::trigger("NPC", *event);
        break;

    case GPMSG_NPC_CLOSE:
        event = new Mana::Event("Close");
        event->setInt("id", npcId);
        Mana::EventManager::trigger("NPC", *event);
        break;
    }

    delete event;
}

void NpcHandler::event(const std::string &channel, const Mana::Event &event)
{
    if (channel == "NPC")
    {
        if (event.getName() == "doTalk")
        {
            MessageOut msg(PGMSG_NPC_TALK);
            msg.writeInt16(event.getInt("npcId"));
            gameServerConnection->send(msg);
        }
        else if (event.getName() == "doNext" || event.getName() == "doClose")
        {
            MessageOut msg(PGMSG_NPC_TALK_NEXT);
            msg.writeInt16(event.getInt("npcId"));
            gameServerConnection->send(msg);
        }
        else if (event.getName() == "doMenu")
        {
            MessageOut msg(PGMSG_NPC_SELECT);
            msg.writeInt16(event.getInt("npcId"));
            msg.writeInt8(event.getInt("choice"));
            gameServerConnection->send(msg);
        }
        else if (event.getName() == "doIntegerInput")
        {
            MessageOut msg(PGMSG_NPC_NUMBER);
            msg.writeInt16(event.getInt("npcId"));
            msg.writeInt32(event.getInt("value"));
            gameServerConnection->send(msg);
        }
        else if (event.getName() == "doStringInput")
        {
            MessageOut msg(PGMSG_NPC_STRING);
            msg.writeInt16(event.getInt("npcId"));
            msg.writeString(event.getString("value"));
            gameServerConnection->send(msg);
        }
        else if (event.getName() == "doSendLetter")
        {
            MessageOut msg(PGMSG_NPC_POST_SEND);
            msg.writeString(event.getString("recipient"));
            msg.writeString(event.getString("text"));
            gameServerConnection->send(msg);
        }
    }
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

} // namespace ManaServ
