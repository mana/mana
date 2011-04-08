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

#ifndef NET_MESSAGEIN_H
#define NET_MESSAGEIN_H

#include <SDL_types.h>

#include <string>

namespace Net {

/**
 * Used for parsing an incoming message.
 *
 * \ingroup Network
 */
class MessageIn
{
    public:
        /**
         * Returns the message ID.
         */
        int getId() const { return mId; }

        /**
         * Returns the message length.
         */
        unsigned int getLength() const { return mLength; }

        /**
         * Returns the length of unread data.
         */
        unsigned int getUnreadLength() const { return mLength - mPos; }

        virtual int readInt8();             /**< Reads a byte. */
        virtual int readInt16() = 0;        /**< Reads a short. */
        virtual int readInt32() = 0;        /**< Reads a long. */

        /**
         * Reads a 3-byte block containing tile-based coordinates. Used by
         * manaserv.
         */
        virtual void readCoordinates(Uint16 &x, Uint16 &y);

        /**
         * Reads a special 3 byte block used by eAthena, containing x and y
         * coordinates and direction.
         */
        virtual void readCoordinates(Uint16 &x, Uint16 &y, Uint8 &direction);

        /**
         * Reads a special 5 byte block used by eAthena, containing a source
         * and destination coordinate pair.
         */
        virtual void readCoordinatePair(Uint16 &srcX, Uint16 &srcY,
                                        Uint16 &dstX, Uint16 &dstY);

        /**
         * Skips a given number of bytes.
         */
        virtual void skip(unsigned int length);

        /**
         * Reads a string. If a length is not given (-1), it is assumed
         * that the length of the string is stored in a short at the
         * start of the string.
         */
        virtual std::string readString(int length = -1);

        virtual ~MessageIn() {}

    protected:
        MessageIn(const char *data, unsigned int length);

        const char *mData;             /**< The message data. */
        unsigned int mLength;          /**< The length of the data. */
        unsigned short mId;            /**< The message ID. */

        /**
         * Actual position in the packet. From 0 to packet->length.
         * A value bigger than packet->length means EOP was reached when
         * reading it.
         */
        unsigned int mPos;
};

}

#endif // NET_MESSAGEIN_H
