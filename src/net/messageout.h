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

#ifndef MESSAGEOUT_H
#define MESSAGEOUT_H

#include <iosfwd>
#include <SDL_types.h>

#ifdef EATHENA_SUPPORT
class Network;
#endif

/**
 * Used for building an outgoing message.
 *
 * With tmwserv, the message is sent using Net::Connection::send() when
 * finished.
 *
 * \ingroup Network
 */
class MessageOut
{
    public:
        /**
         * Constructor.
         */
        MessageOut(short id);

#ifdef TMWSERV_SUPPORT
        /**
         * Destructor.
         */
        ~MessageOut();
#endif

        void writeInt8(Sint8 value);          /**< Writes a byte. */
        void writeInt16(Sint16 value);        /**< Writes a short. */
        void writeInt32(Sint32 value);        /**< Writes a long. */

#ifdef EATHENA_SUPPORT
        /**
         * Encodes coordinates and direction in 3 bytes. Used by eAthena.
         */
        void writeCoordinates(unsigned short x, unsigned short y,
                              unsigned char direction);
#endif

        /**
         * Writes a string. If a fixed length is not given (-1), it is stored
         * as a short at the start of the string.
         */
        void writeString(const std::string &string, int length = -1);

        /**
         * Returns the content of the message.
         */
        char *getData() const;

        /**
         * Returns the length of the data.
         */
        unsigned int getDataSize() const;

    private:
#ifdef TMWSERV_SUPPORT
        /**
         * Expand the packet data to be able to hold more data.
         *
         * NOTE: For performance enhancements this method could allocate extra
         * memory in advance instead of expanding size every time more data is
         * added.
         */
        void expand(size_t size);
#else
        Network *mNetwork;
#endif

        char *mData;                         /**< Data building up. */
        unsigned int mDataSize;              /**< Size of data. */
        unsigned int mPos;                   /**< Position in the data. */
};

#endif
