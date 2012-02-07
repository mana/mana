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

    typedef std::pair<int, std::string> VersionString;

    Type type;
    std::string name;
    std::string hostname;
    unsigned short port;

    std::string description;
    VersionString version;

    bool save;

    ServerInfo()
    {
        type = UNKNOWN;
        port = 0;
        save = false;
        version.first = 0;
    }

    ServerInfo(const ServerInfo &info)
    {
        type = info.type;
        name = info.name;
        hostname = info.hostname;
        port = info.port;
        description = info.description;
        version.first = info.version.first;
        version.second = info.version.second;
        save = info.save;
    }

    bool isValid() const
    {
        return !(hostname.empty() || port == 0 || type == UNKNOWN);
    }

    void clear()
    {
        type = UNKNOWN;
        name.clear();
        hostname.clear();
        port = 0;
        description.clear();
        version.first = 0;
        version.second.clear();
        save = false;
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
};

typedef std::deque<ServerInfo> ServerInfos;

#endif // SERVERINFO_H
