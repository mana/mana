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

#ifndef _TMW_SELL_H
#define _TMW_SELL_H

#include "window.h"
#include "shop.h"

#include <vector>

/**
 * The sell dialog.
 *
 * \ingroup Interface
 */
class SellDialog : public Window, public gcn::ActionListener,
                  public gcn::ListModel
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        SellDialog();

        /**
         * Destructor.
         */
        ~SellDialog();

        /**
         * Resets the dialog, clearing inventory.
         */
        void reset();

        /**
         * Adds an item to the inventory.
         */
        void addItem(short id, int price);

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string& eventId);

        /**
         * Returns the number of items in the inventory.
         */
        int getNumberOfElements();

        /**
         * Returns the name of item number i in the inventory.
         */
        std::string getElementAt(int i);

    private:
        gcn::Button *okButton;
        gcn::Button *cancelButton;
        gcn::ListBox *itemList;
        gcn::ScrollArea *scrollArea;
        gcn::Label *moneyLabel;
        gcn::Label *quantityLabel;
        gcn::Slider *slider;

        std::vector<ITEM_SHOP> shopInventory;

        int maxItems;
};

#endif
