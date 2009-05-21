/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef PLAYERHANDLER_H
#define PLAYERHANDLER_H

#include "being.h"
#include "flooritem.h"
#include "localplayer.h"

namespace Net {

class PlayerHandler
{
    public:
        virtual void attack(Being *being) = 0;

        virtual void emote(int emoteId) = 0;

        virtual void increaseStat(LocalPlayer::Attribute attr) = 0;

        virtual void decreaseStat(LocalPlayer::Attribute attr) = 0;

        virtual void pickUp(FloorItem *floorItem) = 0;

        virtual void setDirection(char direction) = 0;

        virtual void setDestination(int x, int y, int direction = -1) = 0;

        virtual void changeAction(Being::Action action) = 0;

        virtual void respawn() = 0;

        virtual void ignorePlayer(const std::string &player, bool ignore) = 0;

        virtual void ignoreAll(bool ignore) = 0;
};

} // namespace Net

#endif // PLAYERHANDLER_H
