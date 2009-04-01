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
#include "floor_item.h"
#include "localplayer.h"

#include <iosfwd>

namespace Net {
class PlayerHandler
{
    public:
        virtual void attack(Being *being) {}

        virtual void emote(int emoteId) {}

        virtual void increaseStat(LocalPlayer::Attribute attr) {}

        virtual void decreaseStat(LocalPlayer::Attribute attr) {}

        virtual void pickUp(FloorItem *floorItem) {}

        virtual void setDirection(int direction) {}

        virtual void setDestination(int x, int y, int direction = -1) {}

        virtual void changeAction(Being::Action action) {}

        virtual void respawn() {}

        virtual void ingorePlayer(const std::string &player, bool ignore) {}

        virtual void ingoreAll(bool ignore) {}
};
}

#endif // PLAYERHANDLER_H
