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

#pragma once

#include "net/manaserv/messagehandler.h"

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

class BeingHandler final : public MessageHandler
{
    public:
        BeingHandler();

        void handleMessage(MessageIn &msg) override;

    private:
        void handleBeingAttackMessage(MessageIn &msg);
        void handleBeingEnterMessage(MessageIn &msg);
        void handleBeingLeaveMessage(MessageIn &msg);
        void handleBeingsMoveMessage(MessageIn &msg);
        void handleBeingsDamageMessage(MessageIn &msg);
        void handleBeingActionChangeMessage(MessageIn &msg);
        void handleBeingLooksChangeMessage(MessageIn &msg);
        void handleBeingDirChangeMessage(MessageIn &msg);
};

} // namespace ManaServ
