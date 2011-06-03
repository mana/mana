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

MessageOut::MessageOut(Uint16 id):
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

void MessageOut::writeInt16(Uint16 value)
{
    expand(2);
    mData[mPos] = value;
    mData[mPos + 1] = value >> 8;
    mPos += 2;
}

void MessageOut::writeInt32(Uint32 value)
{
    expand(4);
    mData[mPos] = value;
    mData[mPos + 1] = value >> 8;
    mData[mPos + 2] = value >> 16;
    mData[mPos + 3] = value >> 24;
    mPos += 4;
}

void MessageOut::writeCoordinates(Uint16 x, Uint16 y, Uint8 direction)
{
    char *data = mData + mPos;
    mNetwork->mOutSize += 3;
    mPos += 3;

    Uint16 temp = x;
    temp <<= 6;
    data[0] = temp >> 8;
    data[1] = temp;

    temp = y;
    temp <<= 4;
    data[1] |= temp << 8;
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
