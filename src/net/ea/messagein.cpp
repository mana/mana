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

#include "net/ea/messagein.h"

#include <SDL.h>
#include <SDL_endian.h>

#define MAKEWORD(low,high) \
    ((unsigned short)(((unsigned char)(low)) | \
    ((unsigned short)((unsigned char)(high))) << 8))

namespace EAthena  {

MessageIn::MessageIn(const char *data, unsigned int length):
        Net::MessageIn(data, length)
{
    // Read the message ID
    mId = readInt16();
}

int MessageIn::readInt16()
{
    int value = -1;
    if (mPos + 2 <= mLength)
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        Sint16 swap;
        memcpy(&swap, mData + mPos, sizeof(Sint16));
        value = SDL_Swap16(swap);
#else
        value = (*(Sint16*)(mData + mPos));
#endif
    }
    mPos += 2;
    return value;
}

int MessageIn::readInt32()
{
    int value = -1;
    if (mPos + 4 <= mLength)
    {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        Sint32 swap;
        memcpy(&swap, mData + mPos, sizeof(Sint32));
        value = SDL_Swap32(swap);
#else
        value = (*(Sint32*)(mData + mPos));
#endif
    }
    mPos += 4;
    return value;
}

}
