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
 *  $Id$
 */


#include "guild.h"

#include "internal.h"

#include "../connection.h"
#include "../messageout.h"
#include "../protocol.h"

#include "../../log.h"

void Net::ChatServer::Guild::createGuild(const std::string &name)
{
    logger->log("Sending PCMSG_GUILD_CREATE");
    MessageOut msg(PCMSG_GUILD_CREATE);

    msg.writeString(name);

    Net::ChatServer::connection->send(msg);
}

void Net::ChatServer::Guild::invitePlayer(const std::string &name, short guildId)
{
    logger->log("Sending PCMSG_GUILD_INVITE");
    MessageOut msg(PCMSG_GUILD_INVITE);

    msg.writeInt16(guildId);
    msg.writeString(name);

    Net::ChatServer::connection->send(msg);
}

void Net::ChatServer::Guild::acceptInvite(const std::string &name)
{
    logger->log("Sending PCMSG_GUILD_ACCEPT");
    MessageOut msg(PCMSG_GUILD_ACCEPT);

    msg.writeString(name);

    Net::ChatServer::connection->send(msg);
}

void Net::ChatServer::Guild::getGuildMembers(short guildId)
{
    logger->log("Sending PCMSG_GUILD_GET_MEMBERS");
    MessageOut msg(PCMSG_GUILD_GET_MEMBERS);

    msg.writeInt16(guildId);

    Net::ChatServer::connection->send(msg);
}

void Net::ChatServer::Guild::quitGuild(short guildId)
{
    logger->log("Sending PCMSG_GUILD_QUIT");
    MessageOut msg(PCMSG_GUILD_QUIT);

    msg.writeInt16(guildId);

    Net::ChatServer::connection->send(msg);
}
