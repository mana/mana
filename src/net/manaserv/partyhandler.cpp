/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "net/manaserv/partyhandler.h"

#include "event.h"
#include "log.h"
#include "localplayer.h"

#include "gui/socialwindow.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/manaserv_protocol.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <iostream>

#define PARTY_ID 1

extern Net::PartyHandler *partyHandler;

namespace ManaServ {

extern Connection *chatServerConnection;
extern Connection *gameServerConnection;

PartyHandler::PartyHandler():
        mParty(Party::getParty(PARTY_ID))
{
    static const Uint16 _messages[] = {
        GPMSG_PARTY_INVITE_ERROR,
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

    mParty->setName("Party");
}

void PartyHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_PARTY_INVITE_ERROR:
        {
            std::string name = msg.readString();
            SERVER_NOTICE(strprintf(_("Party invite failed, because no player "
                                      "called %s is within the visual range."),
                                    name.c_str()));
        } break;

        case CPMSG_PARTY_INVITE_RESPONSE:
        {
            if (msg.readInt8() == ERRMSG_OK)
            {

            }
        } break;

        case CPMSG_PARTY_INVITED:
        {
            socialWindow->showPartyInvite(msg.readString());
        } break;

        case CPMSG_PARTY_ACCEPT_INVITE_RESPONSE:
        {
            if (msg.readInt8() == ERRMSG_OK)
            {
                player_node->setParty(mParty);
                while (msg.getUnreadLength())
                {
                    std::string name = msg.readString();
                    mParty->addMember(0, name);
                }
            }
            else
                SERVER_NOTICE(_("Joining party failed."));
        } break;

        case CPMSG_PARTY_QUIT_RESPONSE:
        {
            if (msg.readInt8() == ERRMSG_OK)
            {
                mParty->clearMembers();
                player_node->setParty(NULL);
            }
        } break;

        case CPMSG_PARTY_NEW_MEMBER:
        {
            int id = msg.readInt32();
            std::string name = msg.readString();

            SERVER_NOTICE(strprintf(_("%s joined the party."),
                                            name.c_str()));

            if (name == player_node->getName())
                player_node->setParty(mParty);

            mParty->addMember(id, name);
        } break;

        case CPMSG_PARTY_MEMBER_LEFT:
        {
            // mParty->removeMember(msg.readString());
        } break;

        case CPMSG_PARTY_REJECTED:
        {
            std::string name = msg.readString();
            SERVER_NOTICE(strprintf(
                        _("%s rejected your invite."), name.c_str()));
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

void PartyHandler::invite(Being *being)
{
    invite(being->getName());
}

void PartyHandler::invite(const std::string &name)
{
    MessageOut msg(PGMSG_PARTY_INVITE);

    msg.writeString(name);

    gameServerConnection->send(msg);
}

void PartyHandler::inviteResponse(const std::string &inviter, bool accept)
{
    MessageOut msg = MessageOut(accept ? PCMSG_PARTY_ACCEPT_INVITE :
                                PCMSG_PARTY_REJECT_INVITE);

    msg.writeString(inviter);

    chatServerConnection->send(msg);
}

void PartyHandler::leave()
{
    MessageOut msg(PCMSG_PARTY_QUIT);

    chatServerConnection->send(msg);
}

void PartyHandler::kick(Being *being)
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
    //MessageOut msg(PCMSG_GUILD_GET_MEMBERS);

    //msg.writeInt16(guildId);

    //chatServerConnection->send(msg);
}

} // namespace ManaServ
