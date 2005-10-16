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

#include "messageout.h"

#include <string>
#include <SDL.h>
#include <SDL_endian.h>

#include "network.h"
#include "packet.h"

MessageOut::MessageOut():
    mPacket(0),
    mData(0),
    mDataSize(0),
    mPos(0)
{
    mData = out + out_size;
}

MessageOut::~MessageOut()
{
    if (mPacket) {
        delete mPacket;
    }

    // Don't free this data for now, see above.
    //if (mData) {
    //    free(mData);
    //}
}

void MessageOut::expand(size_t bytes)
{
    /*mData = (char*)realloc(mData, bytes);
    mDataSize = bytes;*/
}

void MessageOut::writeInt8(Sint8 value)
{
    expand(mPos + sizeof(Sint8));
    mData[mPos] = value;
    mPos += sizeof(Sint8);
    out_size += sizeof(Sint8);
}

void MessageOut::writeInt16(Sint16 value)
{
    expand(mPos + sizeof(Sint16));
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    (*(Sint16 *)(mData + mPos)) = SDL_Swap16(value);
#else
    (*(Sint16 *)(mData + mPos)) = value;
#endif
    mPos += sizeof(Sint16);
    out_size += sizeof(Sint16);
}

void MessageOut::writeInt32(Sint32 value)
{
    expand(mPos + sizeof(Sint32));
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    (*(Sint32 *)(mData + mPos)) = SDL_Swap32(value);
#else
    (*(Sint32 *)(mData + mPos)) = value;
#endif
    mPos += sizeof(Sint32);
    out_size += sizeof(Sint32);
}

void MessageOut::writeString(const std::string &string, int length)
{
    std::string toWrite = string;

    if (length < 0)
    {
        // Write the length at the start if not fixed
        writeInt16(string.length());
        expand(mPos + string.length());
    }
    else
    {
        // Make sure the length of the string is no longer than specified
        toWrite = string.substr(0, length);
        expand(mPos + length);
    }

    // Write the actual string
    memcpy(&mData[mPos], (void*)toWrite.c_str(), toWrite.length());
    mPos += toWrite.length();
    out_size += toWrite.length();

    // Pad remaining space with zeros
    if (length > (int)toWrite.length())
    {
        memset(&mData[mPos], '\0', length - toWrite.length());
        mPos += length - toWrite.length();
        out_size += length - toWrite.length();
    }
}

const Packet *MessageOut::getPacket()
{
    if (!mPacket)
    {
        mPacket = new Packet(mData, mDataSize);
    }

    return mPacket;
}

MessageOut& operator<<(MessageOut &msg, const Sint8 &rhs)
{
    msg.writeInt8(rhs);
    return msg;
}

MessageOut& operator<<(MessageOut &msg, const Sint16 &rhs)
{
    msg.writeInt16(rhs);
    return msg;
}

MessageOut& operator<<(MessageOut &msg, const Sint32 &rhs)
{
    msg.writeInt32(rhs);
    return msg;
}
