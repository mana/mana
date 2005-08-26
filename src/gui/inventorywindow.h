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

#ifndef _TMW_INVENTORYWINDOW_H
#define _TMW_INVENTORYWINDOW_H 

#include "window.h"

#include "../guichanfwd.h"

class Item;
class ItemContainer;

/**
 * Inventory dialog.
 *
 * \ingroup Interface
 */
class InventoryWindow : public Window, gcn::ActionListener
{
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
         * Logic (updates buttons and weight information)
         */
        void logic();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string& eventId);

        void mouseClick(int x, int y, int button, int count);

        void mouseMotion(int mx, int my);

        Item* getItem();

    private:
        void updateWidgets();    /** Updates widgets size/position */
        void updateButtons();    /** Updates button states */

        ItemContainer *items;

        gcn::Button *useButton, *dropButton;
        gcn::ScrollArea *invenScroll;
        gcn::Label *itemNameLabel;
        gcn::Label *itemDescriptionLabel;
        gcn::Label *itemEffectLabel;
        gcn::Label *weightLabel;
};

extern InventoryWindow *inventoryWindow;

#endif
