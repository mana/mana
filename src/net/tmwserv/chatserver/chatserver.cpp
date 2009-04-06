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

#include "chatserver.h"

#include "internal.h"

#include "net/tmwserv/connection.h"
#include "net/tmwserv/protocol.h"

#include "net/messageout.h"

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

void Net::ChatServer::announce(const std::string &text)
{
    MessageOut msg(PCMSG_ANNOUNCE);
    msg.writeString(text);
    connection->send(msg);
}
