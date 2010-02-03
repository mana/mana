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

#ifndef NET_MANASERV_BEINGHANDLER_H
#define NET_MANASERV_BEINGHANDLER_H

#include "net/manaserv/messagehandler.h"

namespace ManaServ {

class BeingHandler : public MessageHandler
{
    public:
        BeingHandler();

        void handleMessage(Net::MessageIn &msg);

        /**
         * Translate a given speed in tiles per seconds
         * into pixels per ticks.
         * Used to optimize Being::logic() calls.
         * @see MILLISECONDS_IN_A_TICKS
         */
        static float giveSpeedInPixelsPerTicks(float speedInTilesPerSeconds);

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
