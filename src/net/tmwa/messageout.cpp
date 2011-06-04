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

#include "net/tmwa/messageout.h"

#include "net/tmwa/network.h"

#include <SDL.h>
#include <SDL_endian.h>

#include <cstring>
#include <string>

namespace TmwAthena {

MessageOut::MessageOut(uint16_t id):
        Net::MessageOut(id)
{
    mNetwork = TmwAthena::Network::instance();
    mData = mNetwork->mOutBuffer + mNetwork->mOutSize;
    writeInt16(id);
}

void MessageOut::expand(size_t bytes)
{
    mNetwork->mOutSize += bytes;
}

void MessageOut::writeInt16(uint16_t value)
{
    expand(2);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    uint16_t swap=SDL_Swap16(value);
    memcpy(mData + mPos, &swap, sizeof(uint16_t));
#else
    memcpy(mData + mPos, &value, sizeof(uint16_t));
#endif
    mPos += 2;
}

void MessageOut::writeInt32(uint32_t value)
{
    expand(4);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    uint32_t swap=SDL_Swap32(value);
    memcpy(mData + mPos, &swap, sizeof(uint32_t));
#else
    memcpy(mData + mPos, &value, sizeof(uint32_t));
#endif
    mPos += 4;
}

void MessageOut::writeCoordinates(uint16_t x, uint16_t y, uint8_t direction)
{
    char *data = mData + mPos;
    mNetwork->mOutSize += 3;
    mPos += 3;

    uint16_t temp = x;
    temp <<= 6;
    data[0] = temp >> 8;
    data[1] = temp;

    temp = y;
    temp <<= 4;
    data[1] |= temp >> 8;
    data[2] = temp;

    // Translate direction to eAthena format
    switch (direction)
    {
        case 1:
            direction = 0;
            break;
        case 3:
            direction = 1;
            break;
        case 2:
            direction = 2;
            break;
        case 6:
            direction = 3;
            break;
        case 4:
            direction = 4;
            break;
        case 12:
            direction = 5;
            break;
        case 8:
            direction = 6;
            break;
        case 9:
            direction = 7;
            break;
        default:
            // OOPSIE! Impossible or unknown
            direction = 15;
    }
    data[2] |= direction;
}

} // namespace TmwAthena
