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
 *  $Id: npchandler.cpp 2883 2006-12-03 17:00:07Z b_lindeijer $
 */

#include "npchandler.h"

#include "messagein.h"
#include "protocol.h"

#include "../beingmanager.h"
#include "../npc.h"

#include "../gui/npclistdialog.h"
#include "../gui/npc_text.h"

extern NpcListDialog *npcListDialog;
extern NpcTextDialog *npcTextDialog;

NPCHandler::NPCHandler()
{
    static const Uint16 _messages[] = {
        SMSG_NPC_CHOICE,
        SMSG_NPC_MESSAGE,
        SMSG_NPC_NEXT,
        SMSG_NPC_CLOSE,
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
            id = msg->readInt32();
            current_npc = dynamic_cast<NPC*>(beingManager->findBeing(id));
            npcListDialog->parseItems(msg->readString(msg->getLength() - 8));
            npcListDialog->setVisible(true);
            break;

        case SMSG_NPC_MESSAGE:
            msg->readInt16();  // length
            id = msg->readInt32();
            current_npc = dynamic_cast<NPC*>(beingManager->findBeing(id));
            npcTextDialog->addText(msg->readString(msg->getLength() - 8));
            npcListDialog->setVisible(false);
            npcTextDialog->setVisible(true);
            break;

         case SMSG_NPC_CLOSE:
            id = msg->readInt32();
            if (current_npc == dynamic_cast<NPC*>(beingManager->findBeing(id)))
            	current_npc = NULL;
            break;

        case SMSG_NPC_NEXT:
            // Next button in NPC dialog, currently unused
            break;
    }
}
