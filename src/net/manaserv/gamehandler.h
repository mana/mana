/*
 *  The Mana Client
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#ifndef NET_MANASERV_MAPHANDLER_H
#define NET_MANASERV_MAPHANDLER_H

#include "net/gamehandler.h"
#include "net/serverinfo.h"

#include "net/manaserv/messagehandler.h"

namespace ManaServ {

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

        void quit(bool reconnectAccount);

        void quit() { quit(false); }

        void ping(int tick);
        
        bool removeDeadBeings() const { return false; }

        void clear();
};

} // namespace ManaServ

#endif // NET_MANASERV_MAPHANDLER_H
