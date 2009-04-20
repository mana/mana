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

#include "net/tmwserv/connection.h"

#include "net/tmwserv/internal.h"

#include "net/messageout.h"

#include "log.h"

#include <string>

Net::Connection::Connection(ENetHost *client):
    mConnection(0), mClient(client)
{
    Net::connections++;
}

Net::Connection::~Connection()
{
    Net::connections--;
}

bool Net::Connection::connect(const std::string &address, short port)
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
    mConnection = enet_host_connect(mClient, &enetAddress, 1);

    if (!mConnection)
    {
        logger->log("Unable to initiate connection to the server.");
        mState = NET_ERROR;
        return false;
    }

    return true;
}

void Net::Connection::disconnect()
{
    if (!mConnection)
        return;

    enet_peer_disconnect(mConnection, 0);
    enet_host_flush(mClient);
    enet_peer_reset(mConnection);

    mConnection = 0;
}

bool Net::Connection::isConnected()
{
    return (mConnection) ?
                    (mConnection->state == ENET_PEER_STATE_CONNECTED) : false;
}

void Net::Connection::send(const MessageOut &msg)
{
    if (!isConnected())
    {
        logger->log("Warning: cannot send message to not connected server!");
        return;
    }

    //logger->log("Sending message of size %d...", msg.getDataSize());

    ENetPacket *packet = enet_packet_create(msg.getData(),
                                            msg.getDataSize(),
                                            ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(mConnection, 0, packet);
}
