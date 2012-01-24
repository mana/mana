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

#ifndef NET_MANASERV_MESSAGEIN_H
#define NET_MANASERV_MESSAGEIN_H

#include "net/messagein.h"

namespace ManaServ {

/**
 * Used for parsing an incoming message.
 *
 * \ingroup Network
 */
class MessageIn : public Net::MessageIn
{
    public:
        MessageIn(const char *data, unsigned int length);

        uint16_t readInt16();
        uint32_t readInt32();
};

}

#endif // NET_MANASERV_MESSAGEIN_H
