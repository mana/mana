/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
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

#include "net/tmwserv/partyhandler.h"

#include "net/tmwserv/protocol.h"

#include "net/tmwserv/chatserver/chatserver.h"
#include "net/tmwserv/chatserver/party.h"

#include "net/messagein.h"

#include "gui/partywindow.h"

#include "gui/widgets/chattab.h"

#include "log.h"
#include "localplayer.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <iostream>

Net::PartyHandler *partyHandler;

namespace TmwServ {

PartyHandler::PartyHandler()
{
    static const Uint16 _messages[] = {
        CPMSG_PARTY_INVITE_RESPONSE,
        CPMSG_PARTY_INVITED,
        CPMSG_PARTY_ACCEPT_INVITE_RESPONSE,
        CPMSG_PARTY_QUIT_RESPONSE,
        CPMSG_PARTY_NEW_MEMBER,
        CPMSG_PARTY_MEMBER_LEFT,
        CPMSG_PARTY_REJECTED,
        0
    };
    handledMessages = _messages;
    partyHandler = this;
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
                localChatTab->chatLog(_("Joined party."));
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
            int id = msg.readInt16(); // being id
            std::string name = msg.readString();

            localChatTab->chatLog(strprintf(_("%s joined the "
                    "party."), name.c_str()));

            if (!player_node->isInParty())
                player_node->setInParty(true);

            partyWindow->updateMember(id, name);
        } break;

        case CPMSG_PARTY_MEMBER_LEFT:
        {
            partyWindow->removeMember(msg.readString());
        } break;

        case CPMSG_PARTY_REJECTED:
        {
            std::string name = msg.readString();
            localChatTab->chatLog(name + "rejected your invite.");
        } break;
    }
}

void PartyHandler::create(const std::string &name)
{
    // TODO
}

void PartyHandler::join(int partyId)
{
    // TODO
}

void PartyHandler::invite(Player *player)
{
    invite(player->getName());
}

void PartyHandler::invite(const std::string &name)
{
    Net::ChatServer::Party::invitePlayer(name);
}

void PartyHandler::inviteResponse(const std::string &inviter, bool accept)
{
    if (accept)
        Net::ChatServer::Party::acceptInvite(inviter);
    else
        Net::ChatServer::Party::rejectInvite(inviter);
}

void PartyHandler::leave()
{
    Net::ChatServer::Party::quitParty();
}

void PartyHandler::kick(Player *player)
{
    // TODO
}

void PartyHandler::kick(const std::string &name)
{
    // TODO
}

void PartyHandler::chat(const std::string &text)
{
    // TODO
}

void PartyHandler::requestPartyMembers()
{
    // TODO
}

} // namespace TmwServ
