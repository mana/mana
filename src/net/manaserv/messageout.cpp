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

#include "net/manaserv/messageout.h"

#include <enet/enet.h>

#include <cstring>

namespace ManaServ {

MessageOut::MessageOut(uint16_t id):
    mData(0),
    mPos(0),
    mDataSize(0),
    mDebugMode(false)
{
    bool debug = true;
    if (debug)
        id |= ManaServ::XXMSG_DEBUG_FLAG;

    writeInt16(id);
    mDebugMode = debug;
}

MessageOut::~MessageOut()
{
    free(mData);
}

void MessageOut::expand(size_t bytes)
{
    mData = (char*)realloc(mData, mPos + bytes);
    mDataSize = mPos + bytes;
}

void MessageOut::writeInt8(uint8_t value)
{
    if (mDebugMode)
        writeValueType(ManaServ::Int8);

    expand(1);
    mData[mPos] = value;
    mPos += 1;
}

void MessageOut::writeInt16(uint16_t value)
{
    if (mDebugMode)
        writeValueType(ManaServ::Int16);

    expand(2);
    uint16_t t = ENET_HOST_TO_NET_16(value);
    memcpy(mData + mPos, &t, 2);
    mPos += 2;
}

void MessageOut::writeInt32(uint32_t value)
{
    if (mDebugMode)
        writeValueType(ManaServ::Int32);

    expand(4);
    uint32_t t = ENET_HOST_TO_NET_32(value);
    memcpy(mData + mPos, &t, 4);
    mPos += 4;
}

void MessageOut::writeString(const std::string &string, int length)
{
    if (mDebugMode)
    {
        writeValueType(ManaServ::String);
        writeInt16(length);
    }

    int stringLength = string.length();
    if (length < 0)
    {
        // Write the length at the start if not fixed
        writeInt16(stringLength);
        length = stringLength;
    }
    else if (length < stringLength)
    {
        // Make sure the length of the string is no longer than specified
        stringLength = length;
    }
    expand(length);

    // Write the actual string
    memcpy(mData + mPos, string.data(), stringLength);

    if (length > stringLength)
    {
        // Pad remaining space with zeros
        memset(mData + mPos + stringLength, '\0', length - stringLength);
    }
    mPos += length;
}

void MessageOut::writeValueType(ManaServ::ValueType type)
{
    expand(1);
    mData[mPos] = type;
    mPos += 1;
}

} // namespace ManaServ
