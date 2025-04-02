/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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
#include <deque>
#include <string>

enum class ServerType
{
    Unknown,
    ManaServ,
    TmwAthena
};

class ServerInfo
{
public:
    using VersionString = std::pair<int, std::string>;

    ServerType type = ServerType::Unknown;
    std::string name;
    std::string hostname;
    uint16_t port = 0;

    std::string description;
    VersionString version = std::make_pair(0, std::string());

    bool save = false;
    bool persistentIp = true;

    bool isValid() const
    {
        return !hostname.empty() && port != 0 && type != ServerType::Unknown;
    }

    void clear()
    {
        *this = ServerInfo();
    }

    bool operator==(const ServerInfo &other) const
    {
        return (type == other.type && hostname == other.hostname &&
                port == other.port);
    }

    bool operator!=(const ServerInfo &other) const
    {
        return (type != other.type || hostname != other.hostname ||
                port != other.port);
    }

    static ServerType parseType(const std::string &type)
    {
        if (type == "tmwathena")
            return ServerType::TmwAthena;
        // Used for backward compatibility
        if (type == "eathena")
            return ServerType::TmwAthena;
        if (type == "manaserv")
            return ServerType::ManaServ;
        return ServerType::Unknown;
    }

    static uint16_t defaultPortForServerType(ServerType type)
    {
        switch (type)
        {
        default:
        case ServerType::Unknown:
            return 0;
        case ServerType::TmwAthena:
            return 6901;
        case ServerType::ManaServ:
            return 9601;
        }
    }

    static ServerType defaultServerTypeForPort(uint16_t port)
    {
        if (port == 6901)
            return ServerType::TmwAthena;
        if (port == 9601)
            return ServerType::ManaServ;
        return ServerType::Unknown;
    }
};

using ServerInfos = std::deque<ServerInfo>;
