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

#ifndef NET_MANASERV_MESSAGEOUT_H
#define NET_MANASERV_MESSAGEOUT_H

#include "net/messageout.h"

namespace ManaServ {

class MessageOut : public Net::MessageOut
{
    public:
        /**
         * Constructor.
         */
        MessageOut(short id);

        /**
         * Destructor.
         */
        ~MessageOut();

        void writeInt16(Sint16 value);        /**< Writes a short. */
        void writeInt32(Sint32 value);        /**< Writes a long. */

    protected:
        /**
         * Expand the packet data to be able to hold more data.
         *
         * NOTE: For performance enhancements this method could allocate extra
         * memory in advance instead of expanding size every time more data is
         * added.
         */
        void expand(size_t size);
};

}

#endif // NET_MANASERV_MESSAGEOUT_H
