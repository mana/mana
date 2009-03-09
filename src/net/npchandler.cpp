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

#include <SDL_types.h>

#include "messagein.h"
#include "npchandler.h"
#include "protocol.h"

#include "../beingmanager.h"
#include "../localplayer.h"
#include "../npc.h"

#include "../gui/npc_text.h"
#include "../gui/npcintegerdialog.h"
#include "../gui/npclistdialog.h"
#include "../gui/npcstringdialog.h"

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
}

void NPCHandler::handleMessage(MessageIn *msg)
{
    int id;

    switch (msg->getId())
    {
        case SMSG_NPC_CHOICE:
            msg->readInt16();  // length
            current_npc = msg->readInt32();
            player_node->setAction(LocalPlayer::STAND);
            npcListDialog->parseItems(msg->readString(msg->getLength() - 8));
            npcListDialog->setVisible(true);
            npcListDialog->requestFocus();
            break;

        case SMSG_NPC_MESSAGE:
            msg->readInt16();  // length
            current_npc = msg->readInt32();
            player_node->setAction(LocalPlayer::STAND);
            npcTextDialog->addText(msg->readString(msg->getLength() - 8));
            npcTextDialog->setVisible(true);
            break;

         case SMSG_NPC_CLOSE:
            id = msg->readInt32();
            // If we're talking to that NPC, show the close button
            if (id == current_npc)
                npcTextDialog->showCloseButton();
            // Otherwise, move on as an empty dialog doesn't help
            else
                npcTextDialog->closeDialog(id);
            break;

        case SMSG_NPC_NEXT:
            id = msg->readInt32();
            // If we're talking to that NPC, show the next button
            if (id == current_npc)
                npcTextDialog->showNextButton();
            // Otherwise, move on as an empty dialog doesn't help
            else
                npcTextDialog->nextDialog(id);
            break;

        case SMSG_NPC_INT_INPUT:
            // Request for an integer
            current_npc = msg->readInt32();
            player_node->setAction(LocalPlayer::STAND);
            npcIntegerDialog->setRange(0, 2147483647);
            npcIntegerDialog->setDefaultValue(0);
            npcIntegerDialog->setVisible(true);
            npcIntegerDialog->requestFocus();
            break;

        case SMSG_NPC_STR_INPUT:
            // Request for a string
            current_npc = msg->readInt32();
            player_node->setAction(LocalPlayer::STAND);
            npcStringDialog->setValue("");
            npcStringDialog->setVisible(true);
            npcStringDialog->requestFocus();
            break;
    }
}
