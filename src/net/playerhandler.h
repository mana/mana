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

#ifndef PLAYERHANDLER_H
#define PLAYERHANDLER_H

#include "being.h"
#include "flooritem.h"
#include "localplayer.h"

namespace Net {

class PlayerHandler
{
    public:
        virtual ~PlayerHandler() {}

        virtual void attack(int id) = 0;

        virtual void emote(int emoteId) = 0;

        virtual void increaseAttribute(int attr) = 0;

        virtual void decreaseAttribute(int attr) = 0;

        virtual void increaseSkill(int skillId) = 0;

        virtual void pickUp(FloorItem *floorItem) = 0;

        virtual void setDirection(char direction) = 0;

        virtual void setDestination(int x, int y, int direction = -1) = 0;

        virtual void changeAction(Being::Action action) = 0;

        virtual void respawn() = 0;

        virtual void ignorePlayer(const std::string &player, bool ignore) = 0;

        virtual void ignoreAll(bool ignore) = 0;

        virtual bool canUseMagic() = 0;

        virtual bool canCorrectAttributes() = 0;

        virtual int getJobLocation() = 0;

        virtual Vector getDefaultWalkSpeed() = 0;
};

} // namespace Net

#endif // PLAYERHANDLER_H
