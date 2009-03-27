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

#include "gui/window.h"

#include "inventory.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/selectionlistener.hpp>

class Item;
class ItemContainer;
class ProgressBar;
class TextBox;

/**
 * Inventory dialog.
 *
 * \ingroup Interface
 */
class InventoryWindow : public Window,
                        public gcn::ActionListener,
                        public gcn::KeyListener,
                        public gcn::SelectionListener
{
    public:
        /**
         * Constructor.
         */
#ifdef TMWSERV_SUPPORT
        InventoryWindow(int invSize = (INVENTORY_SIZE));
#else
        InventoryWindow(int invSize = (INVENTORY_SIZE - 2));
#endif

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

        /**
         * Handles the mouse clicks.
         */
        void mouseClicked(gcn::MouseEvent &event);

#ifdef TMWSERV_SUPPORT
        /**
         * Handles the key presses.
         */
        void keyPressed(gcn::KeyEvent &event);

        /**
         * Handles the key releases.
         */
        void keyReleased(gcn::KeyEvent &event);
#endif

        /**
         * Updates labels to currently selected item.
         */
        void valueChanged(const gcn::SelectionEvent &event);

    private:
        void updateButtons();    /**< Updates button states. */

        ItemContainer *mItems;

        std::string mWeight;
        std::string mSlots;
        int mUsedSlots;
        int mTotalWeight;
        int mMaxWeight;
        gcn::Button *mUseButton;
        gcn::Button *mDropButton;
#ifdef TMWSERV_SUPPORT
        gcn::Button *mSplitButton;
#endif
        gcn::ScrollArea *mInvenScroll;         /**< Inventory Scroll Area. */
        gcn::Label *mWeightLabel;
        gcn::Label *mSlotsLabel;

        ProgressBar *mWeightBar;
        ProgressBar *mSlotsBar;

        int mMaxSlots;

        bool mSplit;
        bool mItemDesc;
};

extern InventoryWindow *inventoryWindow;

#endif
