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

#ifndef NET_MANASERV_BEINGHANDLER_H
#define NET_MANASERV_BEINGHANDLER_H

#include "net/manaserv/messagehandler.h"
#include "vector.h"
#include "map.h"

namespace ManaServ {

/**
 * enum for sprites layers.
 */
enum SpriteLayer
{
    SPRITE_LAYER_BASE = 0,
    SPRITE_LAYER_HAIR,
    FIXED_SPRITE_LAYER_SIZE
};

class BeingHandler : public MessageHandler
{
    public:
        BeingHandler();

        void handleMessage(Net::MessageIn &msg);

    private:
        void handleBeingAttackMessage(Net::MessageIn &msg);
        void handleBeingEnterMessage(Net::MessageIn &msg);
        void handleBeingLeaveMessage(Net::MessageIn &msg);
        void handleBeingsMoveMessage(Net::MessageIn &msg);
        void handleBeingsDamageMessage(Net::MessageIn &msg);
        void handleBeingActionChangeMessage(Net::MessageIn &msg);
        void handleBeingLooksChangeMessage(Net::MessageIn &msg);
        void handleBeingDirChangeMessage(Net::MessageIn &msg);
};

} // namespace ManaServ

#endif
