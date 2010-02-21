/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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
#include <vector>

class ServerInfo
{
public:
    enum Type {
        UNKNOWN,
        MANASERV,
        EATHENA
    };

    Type type;
    std::string name;
    std::string hostname;
    unsigned short port;

    ServerInfo()
    {
        type = UNKNOWN;
        port = 0;
    }

    ServerInfo(const ServerInfo &info)
    {
        type = info.type;
        name = info.name;
        hostname = info.hostname;
        port = info.port;
    }

    void clear()
    {
        type = UNKNOWN;
        name.clear();
        hostname.clear();
        port = 0;
    }

    bool operator==(const ServerInfo &other)
    {
        return (type == other.type && hostname == other.hostname &&
                port == other.port);
    }

    bool operator!=(const ServerInfo &other)
    {
        return (type != other.type || hostname != other.hostname ||
                port != other.port);
    }
};

typedef std::vector<ServerInfo> ServerInfos;

#endif // SERVERINFO_H
