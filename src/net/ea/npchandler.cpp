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

#include "../messagein.h"
#include "npchandler.h"
#include "protocol.h"

#include "../../beingmanager.h"
#include "../../localplayer.h"
#include "../../npc.h"

#include "../../gui/npc_text.h"
#include "../../gui/npcintegerdialog.h"
#include "../../gui/npclistdialog.h"
#include "../../gui/npcstringdialog.h"

extern NpcIntegerDialog *npcIntegerDialog;
extern NpcListDialog *npcListDialog;
extern NpcTextDialog *npcTextDialog;
extern NpcStringDialog *npcStringDialog;

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

void NPCHandler::handleMessage(MessageIn &msg)
{
    int id;

    switch (msg.getId())
    {
        case SMSG_NPC_CHOICE:
            msg.readInt16();  // length
            id = msg.readInt32();
            player_node->setAction(LocalPlayer::STAND);
            current_npc = dynamic_cast<NPC*>(beingManager->findBeing(id));
            npcListDialog->parseItems(msg.readString(msg.getLength() - 8));
            npcListDialog->setVisible(true);
            break;

        case SMSG_NPC_MESSAGE:
            msg.readInt16();  // length
            id = msg.readInt32();
            player_node->setAction(LocalPlayer::STAND);
            current_npc = dynamic_cast<NPC*>(beingManager->findBeing(id));
            npcTextDialog->addText(msg.readString(msg.getLength() - 8));
            npcListDialog->setVisible(false);
            npcTextDialog->setVisible(true);
            break;

         case SMSG_NPC_CLOSE:
            id = msg.readInt32();
            current_npc = dynamic_cast<NPC*>(beingManager->findBeing(id));
            npcTextDialog->showCloseButton();
            break;

        case SMSG_NPC_NEXT:
            // Next button in NPC dialog, currently unused
            id = msg.readInt32();
            current_npc = dynamic_cast<NPC*>(beingManager->findBeing(id));
            npcTextDialog->showNextButton();
            break;

        case SMSG_NPC_INT_INPUT:
            // Request for an integer
            id = msg.readInt32();
            current_npc = dynamic_cast<NPC*>(beingManager->findBeing(id));
            npcIntegerDialog->setRange(0, 2147483647);
            npcIntegerDialog->setDefaultValue(0);
            npcIntegerDialog->setVisible(true);
            npcIntegerDialog->requestFocus();
            break;

        case SMSG_NPC_STR_INPUT:
            // Request for a string
            id = msg.readInt32();
            current_npc = dynamic_cast<NPC*>(beingManager->findBeing(id));
            npcStringDialog->setValue("");
            npcStringDialog->setVisible(true);
            npcStringDialog->requestFocus();
            break;
    }
}
