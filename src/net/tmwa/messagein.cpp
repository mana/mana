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

#include "net/tmwa/messagein.h"

namespace TmwAthena  {

MessageIn::MessageIn(const char *data, unsigned int length):
        Net::MessageIn(data, length)
{
    // Read the message ID
    mId = readInt16();
}

uint16_t MessageIn::readInt16()
{
    uint16_t value = 0;
    if (mPos + 2 <= mLength)
    {
        value = (mData[mPos + 1] << 8) | mData[mPos];
    }
    mPos += 2;
    return value;
}

uint32_t MessageIn::readInt32()
{
    uint32_t value = 0;
    if (mPos + 4 <= mLength)
    {
        value = (mData[mPos + 3] << 24) | (mData[mPos + 2] << 16) | (mData[mPos + 1] << 8) | mData[mPos];
    }
    mPos += 4;
    return value;
}

} // namespace TmwAthena
