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

#include "net/serverstatus.h"

#include "log.h"

#include "net/tmwa/messagein.h"
#include "net/tmwa/messageout.h"
#include "net/tmwa/network.h"
#include "net/tmwa/protocol.h"

using namespace TmwAthena;

ServerStatusChecker::ServerStatusChecker(const ServerInfo &server)
{
    static const uint16_t _messages[] = {
        SMSG_SERVER_VERSION_RESPONSE,
        0
    };
    handledMessages = _messages;

    if (server.type != ServerType::TmwAthena)
        return;

    mNetworkOwner = std::make_unique<Network>();
    mNetworkOwner->registerHandler(this);

    if (!mNetworkOwner->connect(server))
    {
        finish(ServerStatus::State::Offline);
        return;
    }

    mStatus.state = ServerStatus::State::Checking;
    sendMessage(CMSG_SERVER_VERSION_REQUEST);
}

ServerStatusChecker::~ServerStatusChecker() = default;

void ServerStatusChecker::update()
{
    if (!mNetworkOwner)
        return;

    // Also sends a pending disconnect request after the probe finished
    mNetworkOwner->flush();

    if (mStatus.state != ServerStatus::State::Checking)
        return;

    mNetworkOwner->dispatchMessages();

    const int networkState = mNetworkOwner->getState();
    if (networkState == Network::NET_ERROR)
        finish(ServerStatus::State::Offline);
    else if (networkState == Network::IDLE)
        finish(ServerStatus::State::Online);   // server closed the connection
}

void ServerStatusChecker::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
    case SMSG_SERVER_VERSION_RESPONSE:
        sendMessage(CMSG_CLIENT_DISCONNECT);
        finish(ServerStatus::State::Online);
        break;
    }
}

void ServerStatusChecker::finish(ServerStatus::State state)
{
    mStatus.state = state;

    const ServerInfo &server = mNetworkOwner->getServer();
    Log::info("Server status of %s:%d: %s",
              server.hostname.c_str(), server.port,
              state == ServerStatus::State::Online ? "online" : "offline");
}
