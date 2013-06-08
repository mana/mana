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

#ifndef NET_MANASERV_MESSAGEIN_H
#define NET_MANASERV_MESSAGEIN_H

#include "net/manaserv/manaserv_protocol.h"

#include <cstdint>
#include <string>

namespace ManaServ {

/**
 * Used for parsing an incoming message from manaserv.
 *
 * \ingroup Network
 */
class MessageIn
{
    public:
        MessageIn(const char *data, unsigned int length);

        /**
         * Returns the message ID.
         */
        uint16_t getId() const { return mId; }

        /**
         * Returns the message length.
         */
        unsigned int getLength() const { return mLength; }

        /**
         * Returns the length of unread data.
         */
        unsigned int getUnreadLength() const
        {
            return (mPos < mLength) ? mLength - mPos : 0;
        }

        /**
         * Reads an unsigned 8-bit integer from the message.
         */
        uint8_t readInt8();

        /**
         * Reads an unsigned 16-bit integer from the message.
         */
        uint16_t readInt16();

        /**
         * Reads an unsigned 32-bit integer from the message.
         */
        uint32_t readInt32();

        /**
         * Reads a string. If a length is not given (-1), it is assumed
         * that the length of the string is stored in a short at the
         * start of the string.
         */
        std::string readString(int length = -1);

    private:
        bool readValueType(ManaServ::ValueType type);

        const char *mData;            /**< The message data. */
        unsigned int mLength;         /**< The length of the data. */
        unsigned short mId;           /**< The message ID. */
        bool mDebugMode;              /**< Includes debugging information. */

        /**
         * Actual position in the packet. From 0 to packet->length. A value
         * bigger than packet->length means EOP was reached when reading it.
         */
        unsigned int mPos;
};

} // namespace ManaServ

#endif // NET_MANASERV_MESSAGEIN_H
