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

#include "../item.h"

#include "../item.h"
#include "../resources/image.h"
#include "../graphic/spriteset.h"

#define INVENTORY_SIZE 100

/**
 * An item container. Used to show items in inventory and trade dialog.
 *
 * \ingroup GUI
 */
class ItemContainer : public gcn::Widget, public gcn::MouseListener
{
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
         * Sets the width of the container. This is used to determine the new
         * height of the container.
         */
        void setWidth(int width);

        /**
         * Handles mouse click.
         */
        void mousePress(int mx, int my, int button);

        /**
         * Finds the index of an item.
         */
        int getIndex(int id);

        /**
         * Returns the selected item.
         */
        Item* getItem();

        /**
         * Returns the item at the specified index.
         */
        Item* getItem(int index);

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
         * Get the number of slots filled with an item
         */
        int getNumberOfSlotsUsed();

    private:
        Spriteset *itemset;
        Image *selImg;
        Item *selectedItem;
        int itemNumber;
        Item items[INVENTORY_SIZE];  /**< The holder of items */

};

#endif
