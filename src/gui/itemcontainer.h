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
#include "../resources/image.h"
#include "../graphic/spriteset.h"

#define INVENTORY_SIZE 100

struct ITEM_HOLDER { // the holder of a item
    int id;          // the id of the item
    int quantity;    // number of items
};

/**
 * An item container. Used to show items in inventory.
 *
 * \ingroup GUI
 */
class ItemContainer : public gcn::Widget
{
    private:
        
        Spriteset *itemset;
        int selectedItem;
        int itemNumber;
        ITEM_HOLDER items[INVENTORY_SIZE];  /**< this is the holder of items */        

    public:
        /**
         * Constructor. Initializes the graphic.
         *
         * @param path  The file containing item images.
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
         * Adds a new item.
         */
        void addItem(int index, int id, int quantity);

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
        
        void _mouseInputMessage(const gcn::MouseInput &mouseInput);
};

#endif
