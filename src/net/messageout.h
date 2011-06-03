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

#ifndef NET_MESSAGEOUT_H
#define NET_MESSAGEOUT_H

#include <SDL_types.h>

#include <iosfwd>

namespace Net {

/**
 * Used for building an outgoing message.
 *
 * \ingroup Network
 */
class MessageOut
{
    public:
        virtual void writeInt8(Uint8 value);          /**< Writes a byte. */
        virtual void writeInt16(Uint16 value) = 0;    /**< Writes a short. */
        virtual void writeInt32(Uint32 value) = 0;    /**< Writes a "long". */

        /**
         * Writes a string. If a fixed length is not given (-1), it is stored
         * as a short at the start of the string.
         */
        virtual void writeString(const std::string &string, int length = -1);

        /**
         * Returns the content of the message.
         */
        virtual char *getData() const;

        /**
         * Returns the length of the data.
         */
        virtual unsigned int getDataSize() const;

        virtual ~MessageOut() {}

    protected:
        MessageOut(short id);

        /**
         * Expand the packet data to be able to hold more data.
         *
         * NOTE: For performance enhancements this method could allocate extra
         * memory in advance instead of expanding size every time more data is
         * added.
         */
        virtual void expand(size_t size) = 0;

        char *mData;                         /**< Data building up. */
        unsigned int mDataSize;              /**< Size of data. */
        unsigned int mPos;                   /**< Position in the data. */
};

}

#endif // NET_MESSAGEOUT_H
