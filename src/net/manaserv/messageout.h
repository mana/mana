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

#ifndef NET_MANASERV_MESSAGEOUT_H
#define NET_MANASERV_MESSAGEOUT_H

#include "net/manaserv/manaserv_protocol.h"

#include <cstdint>
#include <string>

namespace ManaServ {

/**
 * Used for building an outgoing message to manaserv.
 *
 * \ingroup Network
 */
class MessageOut
{
    public:
        MessageOut(uint16_t id);

        ~MessageOut();

        /**
         * Writes an unsigned 8-bit integer to the message.
         */
        void writeInt8(uint8_t value);

        /**
         * Writes an unsigned 16-bit integer to the message.
         */
        void writeInt16(uint16_t value);

        /**
         * Writes an unsigned 32-bit integer to the message.
         */
        void writeInt32(uint32_t value);

        /**
         * Writes a string. If a fixed length is not given (-1), it is stored
         * as a short at the start of the string.
         */
        void writeString(const std::string &string, int length = -1);

        /**
         * Returns the content of the message.
         */
        char *getData() const { return mData; }

        /**
         * Returns the length of the data.
         */
        unsigned int getDataSize() const { return mDataSize; }

    private:
        /**
         * Expand the packet data to be able to hold more data.
         *
         * NOTE: For performance enhancements this method could allocate extra
         * memory in advance instead of expanding size every time more data is
         * added.
         */
        void expand(size_t size);

        void writeValueType(ManaServ::ValueType type);

        char *mData;                /**< Data building up. */
        unsigned int mPos;          /**< Position in the data. */
        unsigned int mDataSize;     /**< Size of data. */
        bool mDebugMode;            /**< Include debugging information. */
};

} // namespace ManaServ

#endif // NET_MANASERV_MESSAGEOUT_H
