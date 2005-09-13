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
 *
 *  $Id$
 */

#ifndef _TMW_MESSAGEIN_
#define _TMW_MESSAGEIN_

#include <string>

/**
 * Used for parsing an incoming message.
 */
class MessageIn
{
    public:
        /**
         * Constructor.
         */
        MessageIn(const char *data, unsigned int length);

        /**
         * Returns the message ID.
         */
        short
        getId() { return mId; }

        /**
         * Returns the message length.
         */
        unsigned int
        getLength() { return mLength; }

        char readByte();               /**< Reads a byte. */
        short readShort();             /**< Reads a short. */
        long readLong();               /**< Reads a long. */

        /**
         * Reads a special 3 byte block used by eAthena, containing x and y
         * coordinates and direction.
         */
        void
        readCoordinates(unsigned short &x,
                        unsigned short &y,
                        unsigned char &direction);

        /**
         * Reads a special 5 byte block used by eAthena, containing a source
         * and destination coordinate pair.
         */
        void
        readCoordinatePair(unsigned short &srcX, unsigned short &srcY,
                           unsigned short &dstX, unsigned short &dstY);

        /**
         * Skips a given number of bytes.
         */
        void
        skip(unsigned int length);

        /**
         * Reads a string. If a length is not given (-1), it is assumed
         * that the length of the string is stored in a short at the
         * start of the string.
         */
        std::string
        readString(int length = -1);

    private:
        const char* mData;             /**< The message data. */
        unsigned int mLength;          /**< The length of the data. */
        unsigned int mPos;             /**< The position in the data. */
        short mId;                     /**< The message ID. */
};

#endif
