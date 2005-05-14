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

#ifndef _TMW_ITEMCONTAINER_H__
#define _TMW_ITEMCONTAINER_H__

#include <iostream>
#include <guichan.hpp>
#include "../graphics.h"
#include "../resources/image.h"
#include "../graphic/spriteset.h"

#define INVENTORY_SIZE 100

/**
 * The holder of a item.
 */
struct ITEM_HOLDER
{
    int id;          /**< The id of the item */
    int quantity;    /**< The number of items */
    bool equipment;  /**< Whether this item is equipment */
    bool equipped;   /**< Whether this item is equipped */
};

/**
 * An item container. Used to show items in inventory and trade dialog.
 *
 * \ingroup GUI
 */
class ItemContainer : public gcn::Widget, public gcn::MouseListener
{
    private:
        Spriteset *itemset;
        Image *selImg;
        int selectedItem;
        int itemNumber;
        ITEM_HOLDER items[INVENTORY_SIZE];  /**< The holder of items */

    public:
        /**
         * Constructor. Initializes the graphic.
         */
        ItemContainer();

        /**
         * Destructor.
         */
        virtual ~ItemContainer();

        /**
         * Draws the items.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Handles mouse click.
         */
        void mousePress(int mx, int my, int button);

        /**
         * Returns index of the selected item.
         */
        int getIndex();

        /**
         * Finds the index of an item.
         */
        int getIndex(int id);

        /**
         * Returns the id of the selected item.
         */
        int getId();

        /**
         * Returns the id of an item.
         */
        int getId(int index);

        /**
         * Returns the quantity of the selected item.
         */
        int getQuantity();

        /**
         * Returns the quantity of an item.
         */
        int getQuantity(int index);

        /**
         * Returns id of next free slot or -1 if all occupied.
         */
        int getFreeSlot();

        /**
         * Adds a new item.
         */
        void addItem(int index, int id, int quantity, bool equipment);
        
        /**
         * Set selected item to -1.
         */
        void selectNone();
        
        /**
         * Reset all item slots.
         */
        void resetItems();

        /**
         * Remove a item from the inventory.
         */
        void removeItem(int id);

        /**
         * Change quantity of an item.
         */
        void changeQuantity(int index, int quantity);

        /**
         * Increase quantity of an item.
         */
        void increaseQuantity(int index, int quantity);

        /**
         * Returns whether the item at the specified index is equipment.
         */
        bool isEquipment(int index);

        /**
         * Returns whether the item at the specified index is equipped.
         */
        bool isEquipped(int index);

        /**
         * Sets whether the item at the specified index is equipped.
         */
        void setEquipped(int index, bool equipped);

        /**
         * Sets whether the item at the specified index is equipment.
         */
        void setEquipment(int index, bool equipment);

        /**
         * Get the number of slots filled with an item
         */
        int getNumberOfSlotsUsed();
};

#endif
