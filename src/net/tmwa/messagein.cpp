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

#include <SDL.h>
#include <SDL_endian.h>

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
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        uint16_t swap;
        memcpy(&swap, mData + mPos, sizeof(uint16_t));
        value = SDL_Swap16(swap);
#else
        memcpy(&value, mData + mPos, sizeof(uint16_t));
#endif
    }
    mPos += 2;
    return value;
}

uint32_t MessageIn::readInt32()
{
    uint32_t value = 0;
    if (mPos + 4 <= mLength)
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        uint32_t swap;
        memcpy(&swap, mData + mPos, sizeof(uint32_t));
        value = SDL_Swap32(swap);
#else
        memcpy(&value, mData + mPos, sizeof(uint32_t));
#endif
    }
    mPos += 4;
    return value;
}

} // namespace TmwAthena
