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

#ifndef _INVENTORY_H
#define _INVENTORY_H 

#ifdef WIN32
  #pragma warning (disable:4312)
#endif

#include <allegro.h>
#include "../main.h"
#include "../game.h"
#include "../log.h"
#include "../net/network.h"
#include "../graphic/image.h"
#include "gui.h"
#include "window.h"

#define INVENTORY_SIZE 100

struct ITEM_HOLDER { // the holder of a item
    int id;       // the id of the item
    int quantity; // number of items
    //int index;    // item position
};

/**
 * Inventory dialog.
 *
 * \ingroup GUI
 */
class InventoryWindow : public Window {
    public:
        /**
         * Constructor.
         */
        InventoryWindow();

        /**
         * Destructor.
         */
        ~InventoryWindow();

        /**
         * Draws the inventory window.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Add an item the inventory.
         */
        int addItem(int index, int id, int quantity);

        /**
         * Remove a item from the inventory.
         */
        int removeItem(int id);

        /**
         * Change quantity of an item.
         */
        int changeQuantity(int index, int quantity);

        /**
         * Increase quantity of an item.
         */
        int increaseQuantity(int index, int quantity);

        ITEM_HOLDER items[INVENTORY_SIZE];  /**< this is the holder of items */

    private:
        int useItem(int index, int id);
        int dropItem(int index, int amunt);

        Spriteset *itemset;
        int itemMeny, itemMeny_x, itemMeny_y, itemMeny_i;
};

#endif
