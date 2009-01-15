/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <cassert>
#include <SDL.h>
#include <SDL_endian.h>

#include "messagein.h"

#define MAKEWORD(low,high) \
    ((unsigned short)(((unsigned char)(low)) | \
    ((unsigned short)((unsigned char)(high))) << 8))

MessageIn::MessageIn(const char *data, unsigned int length):
    mData(data),
    mLength(length),
    mPos(0)
{
    // Read the message ID
    mId = readInt16();
}

Sint8 MessageIn::readInt8()
{
    assert(mPos < mLength);
    return mData[mPos++];
}

Sint16 MessageIn::readInt16()
{
    assert(mPos + 2 <= mLength);
    mPos += 2;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    return SDL_Swap16(*(Sint16*)(mData + (mPos - 2)));
#else
    return (*(Sint16*)(mData + (mPos - 2)));
#endif
}

Sint32 MessageIn::readInt32()
{
    assert(mPos + 4 <= mLength);
    mPos += 4;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    return SDL_Swap32(*(Sint32*)(mData + (mPos - 4)));
#else
    return (*(Sint32*)(mData + (mPos - 4)));
#endif
}

void MessageIn::readCoordinates(Uint16 &x, Uint16 &y, Uint8 &direction)
{
    assert(mPos + 3 <= mLength);

    const char *data = mData + mPos;
    Sint16 temp;

    temp = MAKEWORD(data[1] & 0x00c0, data[0] & 0x00ff);
    x = temp >> 6;
    temp = MAKEWORD(data[2] & 0x00f0, data[1] & 0x003f);
    y = temp >> 4;

    direction = data[2] & 0x000f;

    // Translate from eAthena format
    switch (direction)
    {
        case 0:
            direction = 1;
            break;
        case 1:
            direction = 3;
            break;
        case 2:
            direction = 2;
            break;
        case 3:
            direction = 6;
            break;
        case 4:
            direction = 4;
            break;
        case 5:
            direction = 12;
            break;
        case 6:
            direction = 8;
            break;
        case 7:
            direction = 9;
            break;
        default:
            // OOPSIE! Impossible or unknown
            direction = 0;
    }

    mPos += 3;
}

void MessageIn::readCoordinatePair(Uint16 &srcX, Uint16 &srcY,
                              Uint16 &dstX, Uint16 &dstY)
{
    assert(mPos + 5 <= mLength);

    const char *data = mData + mPos;
    Sint16 temp;

    temp = MAKEWORD(data[3], data[2] & 0x000f);
    dstX = temp >> 2;

    dstY = MAKEWORD(data[4], data[3] & 0x0003);

    temp = MAKEWORD(data[1], data[0]);
    srcX = temp >> 6;

    temp = MAKEWORD(data[2], data[1] & 0x003f);
    srcY = temp >> 4;

    mPos += 5;
}

void MessageIn::skip(unsigned int length)
{
    assert(mPos + length <= mLength);
    mPos += length;
}

std::string MessageIn::readString(int length)
{
    // Get string length
    if (length < 0) {
        length = readInt16();
    }

    // Make sure the string isn't erroneous
    if (length < 0 || mPos + length > mLength) {
        mPos = mLength + 1;
        return "";
    }

    // Read the string
    char const *stringBeg = mData + mPos;
    char const *stringEnd = (char const *)memchr(stringBeg, '\0', length);
    std::string readString(stringBeg,
                           stringEnd ? stringEnd - stringBeg : length);
    mPos += length;
    return readString;
}

Sint8& operator<<(Sint8 &lhs, MessageIn &msg)
{
    lhs = msg.readInt8();
    return lhs;
}

Sint16& operator<<(Sint16 &lhs, MessageIn &msg)
{
    lhs = msg.readInt16();
    return lhs;
}

Sint32& operator<<(Sint32 &lhs, MessageIn &msg)
{
    lhs = msg.readInt32();
    return lhs;
}
