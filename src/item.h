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

#ifndef _ITEM_H_
#define _ITEM_H_

#include "resources/itemmanager.h"

/**
 * Represents one or more instances of a certain item type.
 */
class Item
{
    public:
        /**
         * Constructor.
         */
        Item(int id = -1, int quantity = 0,
             bool equipment = false, bool equipped = false):
            mId(id),
            mQuantity(quantity),
            mEquipment(equipment),
            mEquipped(equipped)
        {
        }

        /**
         * Destructor.
         */
        ~Item() {}

        /**
         * Sets the item id, identifying the item type.
         */
        void
        setId(int id) { mId = id; }

        /**
         * Returns the item id.
         */
        int
        getId() const { return mId; }

        /**
         * Sets the number of items.
         */
        void
        setQuantity(int quantity) { mQuantity = quantity; }

        /**
         * Increases the number of items by the given amount.
         */
        void
        increaseQuantity(int amount) { mQuantity += amount; }

        /**
         * Returns the number of items.
         */
        int
        getQuantity() const { return mQuantity; }

        /**
         * Sets wether this item is considered equipment.
         */
        void
        setEquipment(bool equipment) { mEquipment = equipment; }

        /**
         * Returns wether this item is considered equipment.
         */
        bool
        isEquipment() const { return mEquipment; }

        /**
         * Sets wether this item is equipped.
         */
        void
        setEquipped(bool equipped) { mEquipped = equipped; }

        /**
         * Returns wether this item is equipped.
         */
        bool
        isEquipped() const { return mEquipped; }

        /**
         * Sets the inventory index of this item.
         */
        void
        setInvIndex(int index) { mInvIndex = index; }

        /**
         * Returns the inventory index of this item.
         */
        int
        getInvIndex() const { return mInvIndex; }

        /**
         * Returns information about this item type.
         */
        ItemInfo*
        getInfo() const { return itemDb->getItemInfo(mId); }

    protected:
        int mId;              /**< Item type id. */
        int mQuantity;        /**< Number of items. */
        bool mEquipment;      /**< Item is equipment. */
        bool mEquipped;       /**< Item is equipped. */
        int mInvIndex;        /**< Inventory index. */
};

#endif
