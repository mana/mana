/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef NET_EA_ADMINHANDLER_H
#define NET_EA_ADMINHANDLER_H

#include "net/adminhandler.h"
#include "net/messagehandler.h"
#include "net/net.h"

namespace EAthena {

class AdminHandler : public MessageHandler, public Net::AdminHandler
{
    public:
        AdminHandler();

        void handleMessage(MessageIn &msg);

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
