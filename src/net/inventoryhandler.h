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

#ifndef INVENTORYHANDLER_H
#define INVENTORYHANDLER_H

#include "inventory.h"
#include "item.h"

#include <iosfwd>

namespace Net {

class InventoryHandler
{
    public:
        virtual ~InventoryHandler() {}

        virtual bool canSplit(const Item *item) = 0;

        // TODO: fix/remove me
        virtual size_t getSize(int type) const = 0;

        virtual bool isWeaponSlot(unsigned int slotTypeId) const = 0;

        virtual bool isAmmoSlot(unsigned int slotTypeId) const = 0;

        virtual unsigned int getVisibleSlotsNumber() const
        { return 0; }
};

} // namespace Net

#endif // INVENTORYHANDLER_H
