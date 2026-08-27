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

#include "net/manaserv/enethost.h"

#include "log.h"

namespace ManaServ
{

static int hostCount = 0;

EnetHost::EnetHost(size_t peerCount)
{
    if (hostCount++ == 0 && enet_initialize() != 0)
        Log::critical("Failed to initialize ENet.");

    mHost = enet_host_create(nullptr, peerCount, 0, 0, 0);
}

EnetHost::~EnetHost()
{
    if (mHost)
        enet_host_destroy(mHost);

    if (--hostCount == 0)
        enet_deinitialize();
}

} // namespace ManaServ
