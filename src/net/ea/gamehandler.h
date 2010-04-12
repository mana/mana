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

#ifndef NET_EA_MAPHANDLER_H
#define NET_EA_MAPHANDLER_H

#include "net/gamehandler.h"
#include "net/net.h"
#include "net/serverinfo.h"

#include "net/ea/messagehandler.h"
#include "net/ea/token.h"

namespace EAthena {

class GameHandler : public MessageHandler, public Net::GameHandler
{
    public:
        GameHandler();

        void handleMessage(Net::MessageIn &msg);

        void connect();

        bool isConnected();

        void disconnect();

        void inGame();

        void mapLoaded(const std::string &mapName);

        void who();

        void quit();

        void ping(int tick);

        bool removeDeadBeings() const { return true; }

        void clear();

        void setMap(const std::string map);

    private:
        std::string mMap;
        int mCharID; /// < Saved for map-server switching
};

} // namespace EAthena

#endif // NET_EA_MAPHANDLER_H
