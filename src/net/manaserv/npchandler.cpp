/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "net/manaserv/npchandler.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/protocol.h"

#include "beingmanager.h"
#include "npc.h"

#include "gui/npcpostdialog.h"
#include "gui/npcdialog.h"

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
    Being *being = beingManager->findBeing(msg.readInt16());
    if (!being || being->getType() != Being::NPC)
    {
        return;
    }

    current_npc = being->getId();
    npcDialog->setNpc(current_npc);

    switch (msg.getId())
    {
        case GPMSG_NPC_CHOICE:
            npcDialog->choiceRequest();
            while (msg.getUnreadLength())
            {
                npcDialog->addChoice(msg.readString());
            }
            break;

        case GPMSG_NPC_NUMBER:
        {
            int min_num = msg.readInt32();
            int max_num = msg.readInt32();
            npcDialog->integerRequest(msg.readInt32(), min_num, max_num);
            break;
        }

        case GPMSG_NPC_STRING:
            npcDialog->textRequest("");
            break;

        case GPMSG_NPC_POST:
            npcDialog->setVisible(false);
            npcPostDialog->clear();
            npcPostDialog->setVisible(true);
            break;

        case GPMSG_NPC_ERROR:
            current_npc = NULL;
            break;

        case GPMSG_NPC_MESSAGE:
            npcDialog->addText(msg.readString(msg.getUnreadLength()));
            npcDialog->showNextButton();
            npcDialog->setVisible(true);
            break;

        case GPMSG_NPC_CLOSE:
            npcDialog->showCloseButton();
            break;
    }
}

void NpcHandler::talk(int npcId)
{
    MessageOut msg(PGMSG_NPC_TALK);
    msg.writeInt16(npcId);
    gameServerConnection->send(msg);
}

void NpcHandler::nextDialog(int npcId)
{
    MessageOut msg(PGMSG_NPC_TALK_NEXT);
    msg.writeInt16(npcId);
    gameServerConnection->send(msg);
}

void NpcHandler::closeDialog(int npcId)
{
    MessageOut msg(PGMSG_NPC_TALK_NEXT);
    msg.writeInt16(npcId);
    gameServerConnection->send(msg);

    npcDialog->setVisible(false);
    npcDialog->setText("");
}

void NpcHandler::listInput(int npcId, int value)
{
    MessageOut msg(PGMSG_NPC_SELECT);
    msg.writeInt16(npcId);
    msg.writeInt8(value);
    gameServerConnection->send(msg);
}

void NpcHandler::integerInput(int npcId, int value)
{
    MessageOut msg(PGMSG_NPC_NUMBER);
    msg.writeInt16(npcId);
    msg.writeInt32(value);
    gameServerConnection->send(msg);
}

void NpcHandler::stringInput(int npcId, const std::string &value)
{
    MessageOut msg(PGMSG_NPC_STRING);
    msg.writeInt16(npcId);
    msg.writeString(value);
    gameServerConnection->send(msg);
}

void NpcHandler::sendLetter(int npcId, const std::string &recipient,
                            const std::string &text)
{
    MessageOut msg(PGMSG_NPC_POST_SEND);
    msg.writeString(recipient);
    msg.writeString(text);
    gameServerConnection->send(msg);
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
