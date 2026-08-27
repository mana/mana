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

#include <enet/enet.h>

#include <cstddef>

namespace ManaServ
{
    /**
     * Owns an ENetHost, and takes care of initializing ENet for the first
     * host and deinitializing it after the last one.
     */
    class EnetHost
    {
        public:
            explicit EnetHost(size_t peerCount);
            ~EnetHost();

            EnetHost(const EnetHost &) = delete;
            EnetHost &operator=(const EnetHost &) = delete;

            ENetHost *get() const { return mHost; }

            explicit operator bool() const { return mHost != nullptr; }

        private:
            ENetHost *mHost = nullptr;
    };
}
