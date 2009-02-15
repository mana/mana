/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef INVENTORYWINDOW_H
#define INVENTORYWINDOW_H

#include <guichan/actionlistener.hpp>
#include <guichan/selectionlistener.hpp>

#include "window.h"

#include "../localplayer.h"

class Item;
class ItemContainer;
class ProgressBar;
class TextBox;

/**
 * Inventory dialog.
 *
 * \ingroup Interface
 */
class InventoryWindow : public Window, gcn::ActionListener,
                                       gcn::SelectionListener
{
    public:
        /**
         * Constructor.
         */
        InventoryWindow(int invSize = (INVENTORY_SIZE - 2));

        /**
         * Destructor.
         */
        ~InventoryWindow();

        /**
         * Logic (updates buttons and weight information).
         */
        void logic();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Returns the selected item.
         */
        Item* getSelectedItem() const;

        void mouseClicked(gcn::MouseEvent &event);

    private:
        void updateButtons();    /**< Updates button states. */

        ItemContainer *mItems;

        std::string mWeight;
        std::string mSlots;
        std::string mUsedSlots;
        Uint32 mTotalWeight, mMaxWeight;
        gcn::Button *mUseButton, *mDropButton;
        gcn::ScrollArea *mInvenScroll;

        gcn::Label *mWeightLabel;
        gcn::Label *mSlotsLabel;

        ProgressBar *mWeightBar;
        ProgressBar *mSlotsBar;

        int mMaxSlots;

        bool mItemDesc;
};

extern InventoryWindow *inventoryWindow;

#endif
