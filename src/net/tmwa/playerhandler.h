/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#ifndef NET_TA_PLAYERHANDLER_H
#define NET_TA_PLAYERHANDLER_H

#include "net/net.h"
#include "net/playerhandler.h"

#include "net/tmwa/messagehandler.h"

namespace TmwAthena {

class PlayerHandler : public MessageHandler, public Net::PlayerHandler
{
    public:
        PlayerHandler();

        void handleMessage(Net::MessageIn &msg);

        void attack(int id);
        void emote(int emoteId);

        void increaseAttribute(int attr);
        void decreaseAttribute(int attr);
        void increaseSkill(int skillId);

        void pickUp(FloorItem *floorItem);
        void setDirection(char direction);
        void setDestination(int x, int y, int direction = -1);
        void changeAction(Being::Action action);

        void respawn();

        void ignorePlayer(const std::string &player, bool ignore);
        void ignoreAll(bool ignore);

        bool canUseMagic();
        bool canCorrectAttributes();

        int getJobLocation();

        Vector getDefaultWalkSpeed();
};

} // namespace TmwAthena

#endif // NET_TA_PLAYERHANDLER_H
