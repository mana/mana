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

#ifndef NET_TA_MESSAGEOUT_H
#define NET_TA_MESSAGEOUT_H

#include "net/messageout.h"

namespace TmwAthena {

class Network;

/**
 * Used for building an outgoing message.
 *
 * \ingroup Network
 */
class MessageOut : public Net::MessageOut
{
    public:
        MessageOut(uint16_t id);

        void writeInt16(uint16_t value);
        void writeInt32(uint32_t value);

        /**
         * Encodes coordinates and direction in 3 bytes.
         */
        void writeCoordinates(uint16_t x, uint16_t y,
                              uint8_t direction);

    private:
        void expand(size_t size);

        Network *mNetwork;
};

}

#endif // NET_TA_MESSAGEOUT_H
