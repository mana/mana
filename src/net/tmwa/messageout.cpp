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

#include "net/tmwa/messageout.h"

#include "net/tmwa/network.h"

#include <SDL_endian.h>

#include <cstring>

namespace TmwAthena {

MessageOut::MessageOut(uint16_t id)
{
#ifdef DEBUG
    logger->log("Sending %s (0x%x)", Network::mInstance->messageName(id), id);
#endif
    writeInt16(id);
}

char *MessageOut::expand(size_t bytes)
{
    Network &net = *Network::mInstance;
    char *data = net.mOutBuffer + net.mOutSize;
    net.mOutSize += bytes;
    return data;
}

void MessageOut::writeInt8(uint8_t value)
{
    *expand(1) = value;
}

void MessageOut::writeInt16(uint16_t value)
{
    value = SDL_SwapLE16(value);
    memcpy(expand(sizeof(uint16_t)), &value, sizeof(uint16_t));
}

void MessageOut::writeInt32(uint32_t value)
{
    value = SDL_SwapLE32(value);
    memcpy(expand(sizeof(uint32_t)), &value, sizeof(uint32_t));
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

    char *data = expand(length);

    // Write the actual string
    memcpy(data, string.data(), stringLength);

    // Pad remaining space with zeros
    if (length > stringLength)
    {
        memset(data + stringLength, '\0', length - stringLength);
    }
}

void MessageOut::writeCoordinates(uint16_t x, uint16_t y, uint8_t direction)
{
    char *data = expand(3);

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
