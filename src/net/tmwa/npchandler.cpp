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
#include "localplayer.h"

#include "gui/npcdialog.h"

#include "net/messagein.h"
#include "net/messageout.h"
#include "net/net.h"
#include "net/npchandler.h"

#include "net/tmwa/protocol.h"

#include <SDL_types.h>

extern Net::NpcHandler *npcHandler;

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
}

void NpcHandler::handleMessage(Net::MessageIn &msg)
{
    if (msg.getId() == SMSG_NPC_CHOICE || msg.getId() == SMSG_NPC_MESSAGE)
    {
        msg.readInt16();  // length
    }

    int npcId = msg.readInt32();
    NpcDialogs::iterator diag = mNpcDialogs.find(npcId);
    NpcDialog *dialog = 0;

    if (diag == mNpcDialogs.end())
    {
        // Empty dialogs don't help
        if (msg.getId() == SMSG_NPC_CLOSE)
        {
            closeDialog(npcId);
            return;
        }
        else if (msg.getId() == SMSG_NPC_NEXT)
        {
            nextDialog(npcId);
            return;
        }
        else
        {
            dialog = new NpcDialog(npcId);
            Wrapper wrap;
            wrap.dialog = dialog;
            mNpcDialogs[npcId] = wrap;
        }
    }
    else
    {
        dialog = diag->second.dialog;
    }

    switch (msg.getId())
    {
        case SMSG_NPC_CHOICE:
            dialog->choiceRequest();
            dialog->parseListItems(msg.readString(msg.getLength() - 8));
            break;

        case SMSG_NPC_MESSAGE:
            dialog->addText(msg.readString(msg.getLength() - 8));
            break;

         case SMSG_NPC_CLOSE:
            // Show the close button
            dialog->showCloseButton();
            break;

        case SMSG_NPC_NEXT:
            // Show the next button
            dialog->showNextButton();
            break;

        case SMSG_NPC_INT_INPUT:
            // Request for an integer
            dialog->integerRequest(0);
            break;

        case SMSG_NPC_STR_INPUT:
            // Request for a string
            dialog->textRequest("");
            break;
    }

    if (player_node->getCurrentAction() != Being::SIT)
        player_node->setAction(Being::STAND);
}

void NpcHandler::talk(int npcId)
{
    MessageOut outMsg(CMSG_NPC_TALK);
    outMsg.writeInt32(npcId);
    outMsg.writeInt8(0); // Unused
}

void NpcHandler::nextDialog(int npcId)
{
    MessageOut outMsg(CMSG_NPC_NEXT_REQUEST);
    outMsg.writeInt32(npcId);
}

void NpcHandler::closeDialog(int npcId)
{
    MessageOut outMsg(CMSG_NPC_CLOSE);
    outMsg.writeInt32(npcId);

    NpcDialogs::iterator it = mNpcDialogs.find(npcId);
    if (it != mNpcDialogs.end())
    {
        (*it).second.dialog->close();
        mNpcDialogs.erase(it);
    }
}

void NpcHandler::listInput(int npcId, int value)
{
    MessageOut outMsg(CMSG_NPC_LIST_CHOICE);
    outMsg.writeInt32(npcId);
    outMsg.writeInt8(value);
}

void NpcHandler::integerInput(int npcId, int value)
{
    MessageOut outMsg(CMSG_NPC_INT_RESPONSE);
    outMsg.writeInt32(npcId);
    outMsg.writeInt32(value);
}

void NpcHandler::stringInput(int npcId, const std::string &value)
{
    MessageOut outMsg(CMSG_NPC_STR_RESPONSE);
    outMsg.writeInt16(value.length() + 9);
    outMsg.writeInt32(npcId);
    outMsg.writeString(value, value.length());
    outMsg.writeInt8(0); // Prevent problems with string reading
}

void NpcHandler::sendLetter(int npcId, const std::string &recipient,
                            const std::string &text)
{
    // TODO
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

void NpcHandler::clearDialogs()
{
    mNpcDialogs.clear();
}

} // namespace TmwAthena
