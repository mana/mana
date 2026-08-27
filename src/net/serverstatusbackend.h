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

#include <cstdint>

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

namespace Net {

/**
 * Backend for probing a server of a specific type, used by
 * ServerStatusChecker.
 */
class ServerStatusBackend
{
    public:
        virtual ~ServerStatusBackend() = default;

        /**
         * Processes the connection and returns the current state. Called
         * regularly until the state is no longer Checking.
         */
        virtual ServerStatus::State update() = 0;
};

} // namespace Net
