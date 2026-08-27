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

#include "net/tmwa/serverstatusbackend.h"

#ifdef MANASERV_SUPPORT
#include "net/manaserv/serverstatusbackend.h"
#endif

ServerStatusChecker::ServerStatusChecker(const ServerInfo &server)
    : mServer(server)
{
    switch (server.type)
    {
    case ServerType::TmwAthena:
        mBackend = std::make_unique<TmwAthena::ServerStatusBackend>(server);
        break;
#ifdef MANASERV_SUPPORT
    case ServerType::ManaServ:
        mBackend = std::make_unique<ManaServ::ServerStatusBackend>(server);
        break;
#endif
    default:
        return;
    }

    mStatus.state = ServerStatus::State::Checking;
    update();
}

ServerStatusChecker::~ServerStatusChecker() = default;

void ServerStatusChecker::update()
{
    if (mStatus.state != ServerStatus::State::Checking)
        return;

    const ServerStatus::State state = mBackend->update();
    if (state == ServerStatus::State::Checking)
        return;

    mStatus.state = state;

    Log::info("Server status of %s:%d: %s",
              mServer.hostname.c_str(), mServer.port,
              state == ServerStatus::State::Online ? "online" : "offline");
}
