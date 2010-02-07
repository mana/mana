/*
 *  The Mana World
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#include "net/ea/messageout.h"

#include "net/ea/network.h"

#include <SDL.h>
#include <SDL_endian.h>

#include <cstring>
#include <string>

namespace EAthena {

MessageOut::MessageOut(short id):
        Net::MessageOut(id)
{
    mNetwork = EAthena::Network::instance();
    mData = mNetwork->mOutBuffer + mNetwork->mOutSize;
    writeInt16(id);
}

void MessageOut::expand(size_t bytes)
{
    mNetwork->mOutSize += bytes;
}

void MessageOut::writeInt16(Sint16 value)
{
    expand(2);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Sint16 swap=SDL_Swap16(value);
    memcpy(mData + mPos, &swap, sizeof(Sint16));
#else
    memcpy(mData + mPos, &value, sizeof(Sint16));
#endif
    mPos += 2;
}

void MessageOut::writeInt32(Sint32 value)
{
    expand(4);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    Sint32 swap=SDL_Swap32(value);
    memcpy(mData + mPos, &swap, sizeof(Sint32));
#else
    memcpy(mData + mPos, &value, sizeof(Sint32));
#endif
    mPos += 4;
}

#define LOBYTE(w)  ((unsigned char)(w))
#define HIBYTE(w)  ((unsigned char)(((unsigned short)(w)) >> 8))

void MessageOut::writeCoordinates(unsigned short x, unsigned short y,
                                  unsigned char direction)
{
    char *data = mData + mPos;
    mNetwork->mOutSize += 3;
    mPos += 3;

    short temp;
    temp = x;
    temp <<= 6;
    data[0] = 0;
    data[1] = 1;
    data[2] = 2;
    data[0] = HIBYTE(temp);
    data[1] = (unsigned char) temp;
    temp = y;
    temp <<= 4;
    data[1] |= HIBYTE(temp);
    data[2] = LOBYTE(temp);

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
            direction = (unsigned char) -1;
    }
    data[2] |= direction;
}

}
