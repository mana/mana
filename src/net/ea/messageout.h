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

#ifndef NET_EA_MESSAGEOUT_H
#define NET_EA_MESSAGEOUT_H

#include "net/messageout.h"

#include <iosfwd>
#include <SDL_types.h>

namespace EAthena {

class Network;

/**
 * Used for building an outgoing message.
 *
 * \ingroup Network
 */
class MessageOut : public Net::MessageOut
{
    public:
        /**
         * Constructor.
         */
        MessageOut(short id);

        void writeInt16(Sint16 value);        /**< Writes a short. */
        void writeInt32(Sint32 value);        /**< Writes a long. */

        /**
         * Encodes coordinates and direction in 3 bytes.
         */
        void writeCoordinates(unsigned short x, unsigned short y,
                              unsigned char direction);

    private:
        void expand(size_t size);

        Network *mNetwork;
};

}

#endif // NET_EA_MESSAGEOUT_H
