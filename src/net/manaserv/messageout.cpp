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

#include "net/manaserv/messageout.h"

#include <enet/enet.h>

#include <cstring>
#include <string>

namespace ManaServ {

MessageOut::MessageOut(Uint16 id):
        Net::MessageOut(id)
{
    writeInt16(id);
}

MessageOut::~MessageOut()
{
    free(mData);
}

void MessageOut::expand(size_t bytes)
{
    mData = (char*)realloc(mData, mPos + bytes);
    mDataSize = mPos + bytes;
}

void MessageOut::writeInt16(Uint16 value)
{
    expand(2);
    uint16_t t = ENET_HOST_TO_NET_16(value);
    memcpy(mData + mPos, &t, 2);
    mPos += 2;
}

void MessageOut::writeInt32(Uint32 value)
{
    expand(4);
    uint32_t t = ENET_HOST_TO_NET_32(value);
    memcpy(mData + mPos, &t, 4);
    mPos += 4;
}

} // namespace ManaServ
