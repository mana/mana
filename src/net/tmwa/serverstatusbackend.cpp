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

#include "net/tmwa/serverstatusbackend.h"

#include "net/tmwa/messagein.h"
#include "net/tmwa/messageout.h"
#include "net/tmwa/network.h"
#include "net/tmwa/protocol.h"

namespace TmwAthena {

ServerStatusBackend::ServerStatusBackend(const ServerInfo &server)
{
    static const uint16_t _messages[] = {
        SMSG_SERVER_VERSION_RESPONSE,
        0
    };
    handledMessages = _messages;

    mNetworkOwner = std::make_unique<Network>();
    mNetworkOwner->registerHandler(this);

    if (!mNetworkOwner->connect(server))
    {
        mState = ServerStatus::State::Offline;
        return;
    }

    sendMessage(CMSG_SERVER_VERSION_REQUEST);
}

ServerStatusBackend::~ServerStatusBackend() = default;

ServerStatus::State ServerStatusBackend::update()
{
    if (mState != ServerStatus::State::Checking)
        return mState;

    mNetworkOwner->dispatchMessages();

    // Sends the version request, or the disconnect request once the probe
    // finished. The server closes the connection in response to the latter,
    // so that the worker thread exits by itself.
    mNetworkOwner->flush();

    const int networkState = mNetworkOwner->getState();
    if (networkState == Network::NET_ERROR)
        mState = ServerStatus::State::Offline;
    else if (networkState == Network::IDLE)
        mState = ServerStatus::State::Online;   // server closed the connection

    return mState;
}

void ServerStatusBackend::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
    case SMSG_SERVER_VERSION_RESPONSE:
        sendMessage(CMSG_CLIENT_DISCONNECT);
        mState = ServerStatus::State::Online;
        break;
    }
}

} // namespace TmwAthena
