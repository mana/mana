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

#ifndef _TMW_INVENTORY_H
#define _TMW_INVENTORY_H 

#include "../log.h"
#include "../net/network.h"
#include "../graphic/spriteset.h"
#include "itemcontainer.h"
#include "gui.h"
#include "window.h"
#include "scrollarea.h"

/**
 * Inventory dialog.
 *
 * \ingroup GUI
 */
class InventoryWindow : public Window, gcn::ActionListener {
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
        int addItem(int index, int id, int quantity, bool equipment);

        /**
         * Remove a item from the inventory.
         */
        int removeItem(int id);
        
        void equipItem(int index);

        /**
         * Unequips an item.
         */
        void unequipItem(int index);
        
        /**
         * Change quantity of an item.
         */
        int changeQuantity(int index, int quantity);

        /**
         * Increase quantity of an item.
         */
        int increaseQuantity(int index, int quantity);
        
        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string& eventId);
        
        int dropItem(int index, int quantity);

        void mouseClick(int x, int y, int button, int count);
	void mouseMotion(int mx, int my);
        
        ItemContainer *items;
        
    private:
        gcn::Button *useButton, *dropButton;
	ScrollArea *invenScroll;
        int useItem(int index, int id);
	void updateWidgets();

};

#endif
