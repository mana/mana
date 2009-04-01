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

#include "net/ea/npchandler.h"

#include "net/ea/protocol.h"

#include "net/messagein.h"
#include "net/messageout.h"
#include "net/net.h"
#include "net/npchandler.h"

#include "beingmanager.h"
#include "localplayer.h"
#include "npc.h"

#include "gui/npc_text.h"
#include "gui/npcintegerdialog.h"
#include "gui/npclistdialog.h"
#include "gui/npcstringdialog.h"

#include <SDL_types.h>

NPCHandler *npcHandler;

NPCHandler::NPCHandler()
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

void NPCHandler::handleMessage(MessageIn &msg)
{
    int id;

    switch (msg.getId())
    {
        case SMSG_NPC_CHOICE:
            msg.readInt16();  // length
            current_npc = msg.readInt32();
            player_node->setAction(LocalPlayer::STAND);
            npcListDialog->parseItems(msg.readString(msg.getLength() - 8));
            npcListDialog->setVisible(true);
            npcListDialog->requestFocus();
            break;

        case SMSG_NPC_MESSAGE:
            msg.readInt16();  // length
            current_npc = msg.readInt32();
            player_node->setAction(LocalPlayer::STAND);
            npcTextDialog->addText(msg.readString(msg.getLength() - 8));
            npcTextDialog->setVisible(true);
            npcTextDialog->requestFocus();
            break;

         case SMSG_NPC_CLOSE:
            id = msg.readInt32();
            // If we're talking to that NPC, show the close button
            if (id == current_npc)
                npcTextDialog->showCloseButton();
            // Otherwise, move on as an empty dialog doesn't help
            else
                npcTextDialog->closeDialog(id);
            break;

        case SMSG_NPC_NEXT:
            id = msg.readInt32();
            // If we're talking to that NPC, show the next button
            if (id == current_npc)
                npcTextDialog->showNextButton();
            // Otherwise, move on as an empty dialog doesn't help
            else
                npcTextDialog->nextDialog(id);
            break;

        case SMSG_NPC_INT_INPUT:
            // Request for an integer
            current_npc = msg.readInt32();
            player_node->setAction(LocalPlayer::STAND);
            npcIntegerDialog->setRange(0, 2147483647);
            npcIntegerDialog->setDefaultValue(0);
            npcIntegerDialog->setVisible(true);
            npcIntegerDialog->requestFocus();
            break;

        case SMSG_NPC_STR_INPUT:
            // Request for a string
            current_npc = msg.readInt32();
            player_node->setAction(LocalPlayer::STAND);
            npcStringDialog->setValue("");
            npcStringDialog->setVisible(true);
            npcStringDialog->requestFocus();
            break;
    }
}

void NPCHandler::talk(int npcId)
{
    MessageOut outMsg(CMSG_NPC_TALK);
    outMsg.writeInt32(npcId);
    outMsg.writeInt8(0); // Unused
}

void NPCHandler::nextDialog(int npcId)
{
    MessageOut outMsg(CMSG_NPC_NEXT_REQUEST);
    outMsg.writeInt32(npcId);
}

void NPCHandler::closeDialog(int npcId)
{
    MessageOut outMsg(CMSG_NPC_CLOSE);
    outMsg.writeInt32(npcId);
}

void NPCHandler::listInput(int npcId, int value)
{
    MessageOut outMsg(CMSG_NPC_LIST_CHOICE);
    outMsg.writeInt32(npcId);
    outMsg.writeInt8(value);
}

void NPCHandler::integerInput(int npcId, int value)
{
    MessageOut outMsg(CMSG_NPC_INT_RESPONSE);
    outMsg.writeInt32(npcId);
    outMsg.writeInt32(value);
}

void NPCHandler::stringInput(int npcId, const std::string &value)
{
    MessageOut outMsg(CMSG_NPC_STR_RESPONSE);
    outMsg.writeInt16(value.length() + 9);
    outMsg.writeInt32(npcId);
    outMsg.writeString(value, value.length());
    outMsg.writeInt8(0); // Prevent problems with string reading
}

void NPCHandler::buy(int beingId)
{

    MessageOut outMsg(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(beingId);
    outMsg.writeInt8(0); // Buy
}

void NPCHandler::sell(int beingId)
{

    MessageOut outMsg(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(beingId);
    outMsg.writeInt8(1); // Sell
}

void NPCHandler::buyItem(int beingId, int itemId, int amount)
{
    MessageOut outMsg(CMSG_NPC_BUY_REQUEST);
    outMsg.writeInt16(8); // One item (length of packet)
    outMsg.writeInt16(amount);
    outMsg.writeInt16(itemId);
}

void NPCHandler::sellItem(int beingId, int itemId, int amount)
{
    MessageOut outMsg(CMSG_NPC_SELL_REQUEST);
    outMsg.writeInt16(8); // One item (length of packet)
    outMsg.writeInt16(itemId + 2);
    outMsg.writeInt16(amount);
}
