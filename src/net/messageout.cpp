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

#ifdef TMWSERV_SUPPORT
MessageOut::MessageOut(short id):
    mData(0),
#else
MessageOut::MessageOut(Network *network):
    mNetwork(network),
#endif
    mDataSize(0),
    mPos(0)
{
#ifdef TMWSERV_SUPPORT
    writeInt16(id);
#else
    mData = mNetwork->mOutBuffer + mNetwork->mOutSize;
#endif
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
