/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#include "net/messageout.h"

#ifdef TMWSERV_SUPPORT
#include <enet/enet.h>
#else
#include "net/ea/network.h"

#include <SDL.h>
#include <SDL_endian.h>
#endif

#include <cstring>
#include <string>

MessageOut::MessageOut(short id):
    mData(0),
    mDataSize(0),
    mPos(0)
{
#ifdef EATHENA_SUPPORT
    mNetwork = Network::instance();
    mData = mNetwork->mOutBuffer + mNetwork->mOutSize;
#endif
    writeInt16(id);
}

#ifdef TMWSERV_SUPPORT
MessageOut::~MessageOut()
{
    free(mData);
}

void MessageOut::expand(size_t bytes)
{
    mData = (char*)realloc(mData, bytes);
    mDataSize = bytes;
}
#endif

void MessageOut::writeInt8(Sint8 value)
{
#ifdef TMWSERV_SUPPORT
    expand(mPos + 1);
#else
    mNetwork->mOutSize += 1;
#endif
    mData[mPos] = value;
    mPos += 1;
}

void MessageOut::writeInt16(Sint16 value)
{
#ifdef TMWSERV_SUPPORT
    expand(mPos + 2);
    uint16_t t = ENET_HOST_TO_NET_16(value);
    memcpy(mData + mPos, &t, 2);
#else
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    (*(Sint16 *)(mData + mPos)) = SDL_Swap16(value);
#else
    (*(Sint16 *)(mData + mPos)) = value;
#endif
    mNetwork->mOutSize += 2;
#endif // TMWSERV_SUPPORT
    mPos += 2;
}

void MessageOut::writeInt32(Sint32 value)
{
#ifdef TMWSERV_SUPPORT
    expand(mPos + 4);
    uint32_t t = ENET_HOST_TO_NET_32(value);
    memcpy(mData + mPos, &t, 4);
#else
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    (*(Sint32 *)(mData + mPos)) = SDL_Swap32(value);
#else
    (*(Sint32 *)(mData + mPos)) = value;
#endif
    mNetwork->mOutSize += 4;
#endif // TMWSERV_SUPPORT
    mPos += 4;
}

#ifdef EATHENA_SUPPORT

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

#endif // EATHENA_SUPPORT

void MessageOut::writeString(const std::string &string, int length)
{
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
#ifdef TMWSERV_SUPPORT
    expand(mPos + length);
#else
    mNetwork->mOutSize += length;
#endif

    // Write the actual string
    memcpy(mData + mPos, string.c_str(), stringLength);

    // Pad remaining space with zeros
    if (length > stringLength)
    {
        memset(mData + mPos + stringLength, '\0', length - stringLength);
    }
    mPos += length;
}

char *MessageOut::getData() const
{
    return mData;
}

unsigned int MessageOut::getDataSize() const
{
    return mDataSize;
}
