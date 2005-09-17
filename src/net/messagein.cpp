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
 *
 *  $Id$
 */

#include "messagein.h"

#include <cassert>
#include <SDL_net.h>
#ifdef SDL_BYTEORDER == SDL_BIG_ENDIAN
#include "win2mac.h"
#endif

#define MAKEWORD(low,high) \
    ((unsigned short)(((unsigned char)(low)) | \
    ((unsigned short)((unsigned char)(high))) << 8))


MessageIn::MessageIn(const char *data, unsigned int length):
    mData(data),
    mLength(length),
    mPos(0)
{
    // Read the message ID
    mId = readShort();
}

char
MessageIn::readByte()
{
    assert(mPos < mLength);
    return mData[mPos++];
}

short
MessageIn::readShort()
{
    assert(mPos + 2 <= mLength);
    mPos += 2;
#ifdef SDL_BYTEORDER == SDL_BIG_ENDIAN
    return DR_SwapTwoBytes(*(short*)(mData + (mPos - 2)));
#else
    return (*(short*)(mData + (mPos - 2)));
#endif
}

long
MessageIn::readLong()
{
    assert(mPos + 4 <= mLength);
    mPos += 4;
#ifdef SDL_BYTEORDER == SDL_BIG_ENDIAN
    return DR_SwapFourBytes(*(long*)(mData + (mPos - 4)));
#else
    return (*(long*)(mData + (mPos - 4)));
#endif
}

void
MessageIn::readCoordinates(unsigned short &x,
                           unsigned short &y,
                           unsigned char &direction)
{
    assert(mPos + 3 <= mLength);

    const char *data = mData + mPos;
    short temp;

    temp = MAKEWORD(data[1] & 0x00c0, data[0] & 0x00ff);
    x = temp >> 6;
    temp = MAKEWORD(data[2] & 0x00f0, data[1] & 0x003f);
    y = temp >> 4;

    direction = data[2] & 0x000f;

    mPos += 3;
}

void
MessageIn::readCoordinatePair(unsigned short &srcX, unsigned short &srcY,
                              unsigned short &dstX, unsigned short &dstY)
{
    assert(mPos + 5 <= mLength);

    const char *data = mData + mPos;
    short temp;

    temp = MAKEWORD(data[3], data[2] & 0x000f);
    dstX = temp >> 2;

    dstY = MAKEWORD(data[4], data[3] & 0x0003);

    temp = MAKEWORD(data[1], data[0]);
    srcX = temp >> 6;

    temp = MAKEWORD(data[2], data[1] & 0x003f);
    srcY = temp >> 4;

    mPos += 5;
}

void
MessageIn::skip(unsigned int length)
{
    assert(mPos + length <= mLength);
    mPos += length;
}

std::string
MessageIn::readString(int length)
{
    int stringLength = 0;
    std::string readString = "";

    // Get string length
    if (length < 0) {
        stringLength = readShort();
    } else {
        stringLength = length;
    }

    // Make sure there is enough data available
    assert(mPos + length <= mLength);

    // Read the string
    char *tmpString = new char[stringLength + 1];
    memcpy(tmpString, (void*)&mData[mPos], stringLength);
    tmpString[stringLength] = 0;
    mPos += stringLength;

    readString = tmpString;
    delete tmpString;

    return readString;
}

char& operator<<(char &lhs, MessageIn &msg)
{
    lhs = msg.readByte();
    return lhs;
}

short& operator<<(short &lhs, MessageIn &msg)
{
    lhs = msg.readShort();
    return lhs;
}

long& operator<<(long &lhs, MessageIn &msg)
{
    lhs = msg.readLong();
    return lhs;
}
