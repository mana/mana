/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "net/manaserv/connection.h"

#include "log.h"

#include "net/manaserv/internal.h"
#include "net/manaserv/messageout.h"

#include <string>

namespace ManaServ
{

Connection::Connection(ENetHost *client):
    mConnection(0), mClient(client)
{
    mPort = 0;
    connections++;
}

Connection::~Connection()
{
    connections--;
}

bool Connection::connect(const std::string &address, short port)
{
    logger->log("Net::Connection::connect(%s, %i)", address.c_str(), port);

    if (address.empty())
    {
        logger->log("Net::Connection::connect() got empty address!");
        mState = NET_ERROR;
        return false;
    }

    ENetAddress enetAddress;

    enet_address_set_host(&enetAddress, address.c_str());
    enetAddress.port = port;

    // Initiate the connection, allocating channel 0.
    mConnection = enet_host_connect(mClient, &enetAddress, 1, 0);

    if (!mConnection)
    {
        logger->log("Unable to initiate connection to the server.");
        mState = NET_ERROR;
        return false;
    }

    mPort = port;

    return true;
}

void Connection::disconnect()
{
    if (!mConnection)
        return;

    enet_peer_disconnect(mConnection, 0);
    enet_host_flush(mClient);
    enet_peer_reset(mConnection);

    mConnection = 0;
}

bool Connection::isConnected()
{
    return (mConnection) ?
                    (mConnection->state == ENET_PEER_STATE_CONNECTED) : false;
}

void Connection::send(const ManaServ::MessageOut &msg)
{
    if (!isConnected())
    {
        logger->log("Warning: cannot send message to not connected server!");
        return;
    }

    ENetPacket *packet = enet_packet_create(msg.getData(),
                                            msg.getDataSize(),
                                            ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(mConnection, 0, packet);
}

} // namespace ManaServ
