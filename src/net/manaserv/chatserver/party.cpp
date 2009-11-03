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

#include "party.h"

#include "internal.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/protocol.h"

#include "log.h"

namespace ManaServ
{

void ChatServer::Party::invitePlayer(const std::string &name)
{
    logger->log("Sending PCMSG_PARTY_INVITE");
    MessageOut msg(PCMSG_PARTY_INVITE);

    msg.writeString(name);

    ChatServer::connection->send(msg);
}

void ChatServer::Party::acceptInvite(const std::string &name)
{
    logger->log("Sending PCMSG_PARTY_ACCEPT_INVITE");
    MessageOut msg(PCMSG_PARTY_ACCEPT_INVITE);

    msg.writeString(name);

    ChatServer::connection->send(msg);
}

void ChatServer::Party::rejectInvite(const std::string &name)
{
    logger->log("Sending PCMSG_PARTY_REJECT_INVITE");
    MessageOut msg(PCMSG_PARTY_REJECT_INVITE);

    msg.writeString(name);

    ChatServer::connection->send(msg);
}

void ChatServer::Party::getPartyMembers()
{
    logger->log("Sending PCMSG_PARTY_GET_MEMBERS");
//    MessageOut msg(PCMSG_GUILD_GET_MEMBERS);

//    msg.writeInt16(guildId);

//    ChatServer::connection->send(msg);
}

void ChatServer::Party::quitParty()
{
    logger->log("Sending PCMSG_PARTY_QUIT");
    MessageOut msg(PCMSG_PARTY_QUIT);

    ChatServer::connection->send(msg);
}

}
