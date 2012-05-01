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

#include "net/manaserv/messagein.h"

#include <cstring>
#include <enet/enet.h>

namespace ManaServ {

MessageIn::MessageIn(const char *data, unsigned int length):
    mData(data),
    mLength(length),
    mDebugMode(false),
    mPos(0)
{
    // Read the message ID
    mId = readInt16();

    // Read and clear the debug flag
    mDebugMode = mId & ManaServ::XXMSG_DEBUG_FLAG;
    mId &= ~ManaServ::XXMSG_DEBUG_FLAG;
}

uint8_t MessageIn::readInt8()
{
    uint8_t value = 0;

    if (!readValueType(ManaServ::Int8))
        return value;

    if (mPos < mLength)
    {
        value = mData[mPos];
    }
    mPos++;
    return value;
}

uint16_t MessageIn::readInt16()
{
    uint16_t value = 0;

    if (!readValueType(ManaServ::Int16))
        return value;

    if (mPos + 2 <= mLength)
    {
        uint16_t t;
        memcpy(&t, mData + mPos, 2);
        value = (unsigned short) ENET_NET_TO_HOST_16(t);
    }
    mPos += 2;
    return value;
}

uint32_t MessageIn::readInt32()
{
    uint32_t value = 0;

    if (!readValueType(ManaServ::Int32))
        return value;

    if (mPos + 4 <= mLength)
    {
        uint32_t t;
        memcpy(&t, mData + mPos, 4);
        value = ENET_NET_TO_HOST_32(t);
    }
    mPos += 4;
    return value;
}

std::string MessageIn::readString(int length)
{
    if (!readValueType(ManaServ::String))
        return std::string();

    if (mDebugMode)
    {
        int fixedLength = (int16_t) readInt16();
        if (fixedLength != length)
        {
            // String does not have the expected length
            mPos = mLength + 1;
            return std::string();
        }
    }

    // Get string length
    if (length < 0)
    {
        length = readInt16();
    }

    // Make sure the string isn't erroneous
    if (length < 0 || mPos + length > mLength)
    {
        mPos = mLength + 1;
        return std::string();
    }

    // Read the string
    const char *stringBeg = mData + mPos;
    const char *stringEnd = (const char *)memchr(stringBeg, '\0', length);
    std::string readString(stringBeg,
                           stringEnd ? stringEnd - stringBeg : length);
    mPos += length;

    return readString;
}

bool MessageIn::readValueType(ManaServ::ValueType type)
{
    if (!mDebugMode) // Verification not possible
        return true;

    if (mPos >= mLength)
        return false;

    uint8_t t = mData[mPos];
    ++mPos;

    return t == type;
}

} // ManaServ
