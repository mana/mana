/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef TRADEHANDLER_H
#define TRADEHANDLER_H

#include "being.h"

#include <iosfwd>

namespace Net {
class TradeHandler
{
    public:
        virtual void request(Being *being) {}

        virtual void respond(bool accept) {}

        virtual void addItem(Item *item, int amount) {}

        virtual void removeItem(int slotNum, int amount) {}

        virtual void setMoney(int amount) {}

        virtual void confirm() {}

        virtual void finish() {}

        virtual void cancel() {}

        virtual ~TradeHandler() {}
};
}

#endif // TRADEHANDLER_H
