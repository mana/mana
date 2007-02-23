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

#include "chatserver.h"

#include "internal.h"

#include "../connection.h"
#include "../messageout.h"
#include "../protocol.h"

using Net::ChatServer::connection;

void Net::ChatServer::connect(Net::Connection *connection,
        const std::string &token)
{
    Net::ChatServer::connection = connection;

    MessageOut msg(PCMSG_CONNECT);

    msg.writeString(token, 32);

    connection->send(msg);
}

void Net::ChatServer::logout()
{
    MessageOut msg(PCMSG_DISCONNECT);

    connection->send(msg);
}

void Net::ChatServer::chat(short channel, const std::string &text)
{
    MessageOut msg(PCMSG_CHAT);

    msg.writeString(text);
    msg.writeShort(channel);

    connection->send(msg);
}

void Net::ChatServer::announce(const std::string &text)
{
    MessageOut msg(PCMSG_ANNOUNCE);

    msg.writeString(text);

    connection->send(msg);
}

void Net::ChatServer::privMsg(const std::string &recipient,
        const std::string &text)
{
    MessageOut msg(PCMSG_PRIVMSG);

    msg.writeString(recipient);
    msg.writeString(text);

    connection->send(msg);
}

void Net::ChatServer::registerChannel(const std::string &name,
        const std::string &annoucement, const std::string &password,
        char isPrivate)
{
    MessageOut msg(PCMSG_REGISTER_CHANNEL);

    msg.writeByte(isPrivate);
    msg.writeString(name);
    msg.writeString(annoucement);
    msg.writeString(password);

    connection->send(msg);
}

void Net::ChatServer::unregisterChannel(short channel)
{
    MessageOut msg(PCMSG_UNREGISTER_CHANNEL);

    msg.writeShort(channel);

    connection->send(msg);
}

void Net::ChatServer::enterChannel(short channel, const std::string &password)
{
    MessageOut msg(PCMSG_ENTER_CHANNEL);

    msg.writeShort(channel);
    msg.writeString(password);

    connection->send(msg);
}

void Net::ChatServer::quitChannel(short channel)
{
    MessageOut msg(PCMSG_QUIT_CHANNEL);

    msg.writeShort(channel);

    connection->send(msg);
}
