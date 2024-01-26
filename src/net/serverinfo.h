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

#ifndef SERVERINFO_H
#define SERVERINFO_H

#include "utils/stringutils.h"

#include <string>
#include <deque>

class ServerInfo
{
public:
    enum Type {
        UNKNOWN,
        MANASERV,
        TMWATHENA
    };

    using VersionString = std::pair<int, std::string>;

    Type type = UNKNOWN;
    std::string name;
    std::string hostname;
    unsigned short port = 0;

    std::string description;
    VersionString version = std::make_pair(0, std::string());

    bool save = false;
    bool persistentIp = true;

    bool isValid() const
    {
        return !(hostname.empty() || port == 0 || type == UNKNOWN);
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

    static Type parseType(const std::string &type)
    {
        if (compareStrI(type, "tmwathena") == 0)
            return TMWATHENA;
        // Used for backward compatibility
        else if (compareStrI(type, "eathena") == 0)
            return TMWATHENA;
        else if (compareStrI(type, "manaserv") == 0)
            return MANASERV;
        return UNKNOWN;
    }

    static unsigned short defaultPortForServerType(Type type)
    {
        switch (type)
        {
            default:
            case ServerInfo::TMWATHENA:
                return 6901;
            case ServerInfo::MANASERV:
                return 9601;
        }
    }
};

using ServerInfos = std::deque<ServerInfo>;

#endif // SERVERINFO_H
