/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#ifndef NET_MANASERV_MAPHANDLER_H
#define NET_MANASERV_MAPHANDLER_H

#include "net/gamehandler.h"
#include "net/serverinfo.h"

#include "net/manaserv/messagehandler.h"

#include "net/manaserv/manaserv_protocol.h"

namespace ManaServ {

class GameHandler : public MessageHandler, public Net::GameHandler
{
    public:
        GameHandler();

        void handleMessage(MessageIn &msg);

        void connect();

        bool isConnected();

        void disconnect();

        void who();

        void quit(bool reconnectAccount);

        void quit() { quit(false); }

        bool removeDeadBeings() const { return false; }

        void clear();

        void gameLoading();

        /** The ManaServ protocol doesn't use the MP status bar. */
        bool canUseMagicBar() const { return false; }

        int getPickupRange() const { return PICKUP_RANGE; }

        int getNpcTalkRange() const { return NPC_TALK_RANGE; }
};

} // namespace ManaServ

#endif // NET_MANASERV_MAPHANDLER_H
