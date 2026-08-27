/*
 *  The Mana Client
 *  Copyright (C) 2026  The Mana Developers
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

#include "net/manaserv/serverstatusbackend.h"

namespace ManaServ {

static constexpr enet_uint32 CONNECT_TIMEOUT_MS = 5000;

ServerStatusBackend::ServerStatusBackend(const ServerInfo &server)
    : mHost(1)
{
    ENetAddress address;
    if (!mHost || enet_address_set_host(&address, server.hostname.c_str()) != 0)
    {
        mState = ServerStatus::State::Offline;
        return;
    }
    address.port = server.port;

    mPeer = enet_host_connect(mHost.get(), &address, 1, 0);
    if (!mPeer)
    {
        mState = ServerStatus::State::Offline;
        return;
    }

    // Give up on the connection attempt sooner than the ENet default
    enet_peer_timeout(mPeer, 0, 0, CONNECT_TIMEOUT_MS);
}

ServerStatusBackend::~ServerStatusBackend()
{
    if (mPeer)
        enet_peer_reset(mPeer);
}

ServerStatus::State ServerStatusBackend::update()
{
    if (mState != ServerStatus::State::Checking)
        return mState;

    ENetEvent event;
    while (enet_host_service(mHost.get(), &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            mState = ServerStatus::State::Online;
            enet_peer_disconnect_now(mPeer, 0);
            mPeer = nullptr;
            return mState;

        case ENET_EVENT_TYPE_DISCONNECT:
            // The connection attempt timed out
            mState = ServerStatus::State::Offline;
            mPeer = nullptr;
            return mState;

        case ENET_EVENT_TYPE_RECEIVE:
            enet_packet_destroy(event.packet);
            break;

        default:
            break;
        }
    }

    return mState;
}

} // namespace ManaServ
