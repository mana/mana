/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _TMW_MESSAGEOUT_
#define _TMW_MESSAGEOUT_

#include <iosfwd>
#include <SDL_types.h>

class Network;

/**
 * Used for building an outgoing message.
 */
class MessageOut
{
    friend MessageOut& operator<<(MessageOut &msg, const Sint8 &rhs);
    friend MessageOut& operator<<(MessageOut &msg, const Sint16 &rhs);
    friend MessageOut& operator<<(MessageOut &msg, const Sint32 &rhs);

    public:
        /**
         * Constructor.
         */
        MessageOut(Network *network);

        void writeInt8(Sint8 value);          /**< Writes a byte. */
        void writeInt16(Sint16 value);        /**< Writes a short. */
        void writeInt32(Sint32 value);          /**< Writes a long. */

        /**
         * Writes a string. If a fixed length is not given (-1), it is stored
         * as a short at the start of the string.
         */
        void writeString(const std::string &string, int length = -1);

    private:
        Network *mNetwork;

        char *mData;                         /**< Data building up. */
        unsigned int mDataSize;              /**< Size of data. */
        unsigned int mPos;                   /**< Position in the data. */
};

#endif
