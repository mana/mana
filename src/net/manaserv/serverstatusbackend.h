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

#pragma once

#include "net/serverinfo.h"
#include "net/serverstatusbackend.h"

#include "net/manaserv/enethost.h"

namespace ManaServ {

/**
 * Probes a manaserv account server by connecting to it. Since ENet
 * connections are established with a handshake, the server is considered
 * online as soon as the connection succeeds.
 */
class ServerStatusBackend : public Net::ServerStatusBackend
{
    public:
        explicit ServerStatusBackend(const ServerInfo &server);
        ~ServerStatusBackend() override;

        ServerStatus::State update() override;

    private:
        EnetHost mHost;
        ENetPeer *mPeer = nullptr;
        ServerStatus::State mState = ServerStatus::State::Checking;
};

} // namespace ManaServ
