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

#pragma once

#include "event.h"

#include "resources/image.h"
#include "resources/itemdb.h"

const int ITEM_ICON_SIZE = 32;

/**
 * Represents one or more instances of a certain item type.
 */
class Item
{
    public:
        Item(int id = -1, int quantity = 0, bool equipped = false);

        virtual ~Item() = default;

        /**
         * Sets the item id, identifying the item type.
         */
        void setId(int id);

        /**
         * Returns the item id.
         */
        int getId() const { return mId; }

        /**
         * Returns the item image.
         */
        Image *getImage() const { return mImage; }

        /**
         * Sets the number of items.
         */
        void setQuantity(int quantity) { mQuantity = quantity; }

        /**
         * Increases the number of items by the given amount.
         */
        void increaseQuantity(int amount) { mQuantity += amount; }

        /**
         * Returns the number of items.
         */
        int getQuantity() const { return mQuantity; }

        /**
         * Sets whether this item is equipped.
         */
        void setEquipped(bool equipped) { mEquipped = equipped; }

        /**
         * Returns whether this item is equipped.
         */
        bool isEquipped() const { return mEquipped; }

        /**
         * Returns whether this item is equippable.
         */
        bool isEquippable() const;

        /**
         * Sets the inventory index of this item.
         */
        void setInvIndex(int index) { mInvIndex = index; }

        /**
         * Returns the inventory index of this item.
         */
        int getInvIndex() const { return mInvIndex; }

        void doEvent(Event::Type eventType);

        void doEvent(Event::Type eventType, int amount);

        /**
         * Returns information about this item type.
         */
        const ItemInfo &getInfo() const { return itemDb->get(mId); }

    protected:
        int mId;                        /**< Item type id. */
        ResourceRef<Image> mImage;      /**< Item image. */
        int mQuantity;                  /**< Number of items. */
        bool mEquipped;                 /**< Item is equipped. */
        int mInvIndex;                  /**< Inventory index. */
};
