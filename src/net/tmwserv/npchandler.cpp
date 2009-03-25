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

#include "npchandler.h"

#include "../messagein.h"
#include "protocol.h"

#include "../../beingmanager.h"
#include "../../npc.h"

#include "../../gui/npclistdialog.h"
#include "../../gui/npcpostdialog.h"
#include "../../gui/npc_text.h"

extern NpcListDialog *npcListDialog;
extern NpcTextDialog *npcTextDialog;
extern NpcPostDialog *npcPostDialog;

NPCHandler::NPCHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_NPC_CHOICE,
        GPMSG_NPC_POST,
        GPMSG_NPC_MESSAGE,
        GPMSG_NPC_ERROR,
        0
    };
    handledMessages = _messages;
}

void NPCHandler::handleMessage(MessageIn &msg)
{
    Being *being = beingManager->findBeing(msg.readInt16());
    if (!being || being->getType() != Being::NPC)
    {
        return;
    }

    current_npc = being->getId();

    switch (msg.getId())
    {
        case GPMSG_NPC_CHOICE:
            npcListDialog->reset();
            while (msg.getUnreadLength())
            {
                npcListDialog->addItem(msg.readString());
            }
            npcListDialog->setVisible(true);
            break;

        case GPMSG_NPC_POST:
            npcTextDialog->setVisible(false);
            npcPostDialog->clear();
            npcPostDialog->setVisible(true);
            break;

        case GPMSG_NPC_ERROR:
            current_npc = NULL;
        case GPMSG_NPC_MESSAGE:
            npcTextDialog->addText(msg.readString(msg.getUnreadLength()));
            npcListDialog->setVisible(false);
            npcTextDialog->setVisible(true);
            npcPostDialog->setVisible(false);
            break;
    }
}
