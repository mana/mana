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

#ifndef NET_EA_PLAYERHANDLER_H
#define NET_EA_PLAYERHANDLER_H

#include "net/messagehandler.h"
#include "net/net.h"
#include "net/playerhandler.h"

namespace EAthena {

class PlayerHandler : public MessageHandler, public Net::PlayerHandler
{
    public:
        PlayerHandler();

        void handleMessage(MessageIn &msg);

        void attack(Being *being);

        void emote(int emoteId);

        void increaseStat(LocalPlayer::Attribute attr);

        void decreaseStat(LocalPlayer::Attribute attr);

        void pickUp(FloorItem *floorItem);

        void setDirection(char direction);

        void setDestination(int x, int y, int direction = -1);

        void changeAction(Being::Action action);

        void respawn();

        void ignorePlayer(const std::string &player, bool ignore);

        void ignoreAll(bool ignore);
};

} // namespace EAthena

#endif // NET_EA_PLAYERHANDLER_H
