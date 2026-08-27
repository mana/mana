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
#include "net/tmwa/messagehandler.h"

#include <cstdint>
#include <memory>

/**
 * The result of probing a server for its status.
 */
struct ServerStatus
{
    enum class State : uint8_t
    {
        Unknown,    ///< Not probed (unsupported server type)
        Checking,   ///< Probe in progress
        Online,
        Offline
    };

    State state = State::Unknown;
};

/**
 * Probes a server to find out whether it is online. The probe does not
 * require authentication.
 *
 * Currently only tmwAthena servers are supported: a version request is sent
 * to the login server, and the server is considered online when it responds.
 *
 * The connection is processed on the main thread by calling update().
 */
class ServerStatusChecker : public TmwAthena::MessageHandler
{
public:
    explicit ServerStatusChecker(const ServerInfo &server);
    ~ServerStatusChecker() override;

    /**
     * Processes the connection. Should be called regularly until the state
     * is no longer ServerStatus::State::Checking.
     */
    void update();

    const ServerStatus &getStatus() const { return mStatus; }

    void handleMessage(TmwAthena::MessageIn &msg) override;

private:
    void finish(ServerStatus::State state);

    std::unique_ptr<TmwAthena::Network> mNetworkOwner;
    ServerStatus mStatus;
};
