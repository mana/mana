/*
 *  The Mana World
 *  Copyright 2008 The Mana World Development Team
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
 */

#include <iostream>
#include "partyhandler.h"

#include "protocol.h"
#include "messagein.h"

#include "chatserver/chatserver.h"

#include "../gui/chat.h"
#include "../gui/partywindow.h"
#include "../log.h"
#include "../localplayer.h"

PartyHandler::PartyHandler()
{
    static const Uint16 _messages[] = {
        CPMSG_PARTY_INVITE_RESPONSE,
        CPMSG_PARTY_INVITED,
        CPMSG_PARTY_ACCEPT_INVITE_RESPONSE,
        CPMSG_PARTY_QUIT_RESPONSE,
        CPMSG_PARTY_NEW_MEMBER,
        CPMSG_PARTY_MEMBER_LEFT,
        0
    };
    handledMessages = _messages;

}

void PartyHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case CPMSG_PARTY_INVITE_RESPONSE:
        {
            if (msg.readInt8() == ERRMSG_OK)
            {

            }
        } break;

        case CPMSG_PARTY_INVITED:
        {
            std::string inviter = msg.readString();
            partyWindow->showPartyInvite(inviter);
        } break;

        case CPMSG_PARTY_ACCEPT_INVITE_RESPONSE:
        {
            if (msg.readInt8() == ERRMSG_OK)
            {
                player_node->setInParty(true);
                chatWindow->chatLog("Joined party");
            }
        }

        case CPMSG_PARTY_QUIT_RESPONSE:
        {
            if (msg.readInt8() == ERRMSG_OK)
            {
                player_node->setInParty(false);
            }
        } break;

        case CPMSG_PARTY_NEW_MEMBER:
        {
            msg.readInt16(); // being id
            std::string name = msg.readString();

            chatWindow->chatLog(name + " joined the party");

            if (!player_node->getInParty())
                player_node->setInParty(true);

            partyWindow->addPartyMember(name);
        } break;

        case CPMSG_PARTY_MEMBER_LEFT:
        {
            partyWindow->removePartyMember(msg.readString());
        } break;
    }
}
