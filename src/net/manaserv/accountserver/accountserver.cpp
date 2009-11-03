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

#include "accountserver.h"

#include "internal.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/protocol.h"

#include "utils/sha256.h"

#include <string>

namespace ManaServ
{

void AccountServer::login(Connection *connection,
        int version, const std::string &username, const std::string &password)
{
    AccountServer::connection = connection;

    MessageOut msg(PAMSG_LOGIN);

    msg.writeInt32(version);
    msg.writeString(username);
    msg.writeString(sha256(username + password));

    AccountServer::connection->send(msg);
}

void AccountServer::registerAccount(Connection *connection,
        int version, const std::string &username, const std::string &password,
        const std::string &email)
{
    AccountServer::connection = connection;

    MessageOut msg(PAMSG_REGISTER);

    msg.writeInt32(version); // client version
    msg.writeString(username);
    // When registering, the password and email hash is assumed by server.
    // Hence, data can be validated safely server-side.
    // This is the only time we send a clear password.
    msg.writeString(password);
    msg.writeString(email);

    AccountServer::connection->send(msg);
}

void AccountServer::logout()
{
    MessageOut msg(PAMSG_LOGOUT);
    AccountServer::connection->send(msg);
}

void AccountServer::reconnectAccount(Connection *connection,
                                               const std::string &passToken)
{
    AccountServer::connection = connection;

    MessageOut msg(PAMSG_RECONNECT);
    msg.writeString(passToken, 32);
    AccountServer::connection->send(msg);
}

}
