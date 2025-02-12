/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "being.h"
#include "net/tmwa/protocol.h"

#include <SDL_endian.h>

#define MAKEWORD(low,high) \
    ((unsigned short)(((unsigned char)(low)) | \
    ((unsigned short)((unsigned char)(high))) << 8))

namespace TmwAthena  {

MessageIn::MessageIn(const char *data, unsigned int length):
    mData(data),
    mLength(length),
    mPos(0)
{
    // Read the message ID
    mId = readInt16();
}

uint8_t MessageIn::readInt8()
{
    uint8_t value = 0;
    if (mPos < mLength)
    {
        value = mData[mPos];
    }
    mPos++;
    return value;
}

uint16_t MessageIn::readInt16()
{
    uint16_t value = 0;
    if (mPos + 2 <= mLength)
    {
        memcpy(&value, mData + mPos, sizeof(uint16_t));
        value = SDL_SwapLE16(value);
    }
    mPos += 2;
    return value;
}

uint32_t MessageIn::readInt32()
{
    uint32_t value = 0;
    if (mPos + 4 <= mLength)
    {
        memcpy(&value, mData + mPos, sizeof(uint32_t));
        value = SDL_SwapLE32(value);
    }
    mPos += 4;
    return value;
}

void MessageIn::readCoordinates(uint16_t &x, uint16_t &y, uint8_t &direction)
{
    if (mPos + 3 <= mLength)
    {
        const char *data = mData + mPos;
        uint16_t temp;

        temp = MAKEWORD(data[1] & 0x00c0, data[0] & 0x00ff);
        x = temp >> 6;
        temp = MAKEWORD(data[2] & 0x00f0, data[1] & 0x003f);
        y = temp >> 4;

        direction = data[2] & 0x000f;

        // Translate from tmwAthena format
        switch (static_cast<DIR>(direction))
        {
            case DIR::S:    direction = Being::DOWN;                break;
            case DIR::SW:   direction = Being::DOWN | Being::LEFT;  break;
            case DIR::W:    direction = Being::LEFT;                break;
            case DIR::NW:   direction = Being::UP | Being::LEFT;    break;
            case DIR::N:    direction = Being::UP;                  break;
            case DIR::NE:   direction = Being::UP | Being::RIGHT;   break;
            case DIR::E:    direction = Being::RIGHT;               break;
            case DIR::SE:   direction = Being::DOWN | Being::RIGHT; break;
            default:
                // OOPSIE! Impossible or unknown
                direction = 0;
                break;
        }
    }
    mPos += 3;
}

void MessageIn::readCoordinatePair(uint16_t &srcX, uint16_t &srcY,
                                   uint16_t &dstX, uint16_t &dstY)
{
    if (mPos + 5 <= mLength)
    {
        const char *data = mData + mPos;
        uint16_t temp;

        temp = MAKEWORD(data[3], data[2] & 0x000f);
        dstX = temp >> 2;

        dstY = MAKEWORD(data[4], data[3] & 0x0003);

        temp = MAKEWORD(data[1], data[0]);
        srcX = temp >> 6;

        temp = MAKEWORD(data[2], data[1] & 0x003f);
        srcY = temp >> 4;
    }
    mPos += 5;
}

void MessageIn::skip(unsigned int length)
{
    mPos += length;
}

std::string MessageIn::readString(int length)
{
    // Get string length
    if (length < 0)
        length = readInt16();

    // Make sure the string isn't erroneous
    if (length < 0 || mPos + length > mLength)
    {
        mPos = mLength + 1;
        return std::string();
    }

    // Read the string
    char const *stringBeg = mData + mPos;
    char const *stringEnd = (char const *)memchr(stringBeg, '\0', length);
    std::string readString(stringBeg,
                           stringEnd ? stringEnd - stringBeg : length);
    mPos += length;
    return readString;
}

} // namespace TmwAthena
