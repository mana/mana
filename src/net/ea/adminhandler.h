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

#ifndef NET_EA_ADMINHANDLER_H
#define NET_EA_ADMINHANDLER_H

#include "net/adminhandler.h"
#include "net/net.h"

#include "net/ea/messagehandler.h"

namespace EAthena {

class AdminHandler : public MessageHandler, public Net::AdminHandler
{
    public:
        AdminHandler();

        void handleMessage(Net::MessageIn &msg);

        void announce(const std::string &text);

        void localAnnounce(const std::string &text);

        void hide(bool hide);

        void kick(int playerId);

        void kick(const std::string &name);

        void ban(int playerId);

        void ban(const std::string &name);

        void unban(int playerId);

        void unban(const std::string &name);

        void mute(int playerId, int type, int limit);
};

} // namespace EAthena

#endif // NET_EA_ADMINHANDLER_H
