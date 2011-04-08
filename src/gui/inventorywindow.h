/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef INVENTORYWINDOW_H
#define INVENTORYWINDOW_H

#include "inventory.h"
#include "listener.h"

#include "gui/widgets/window.h"
#include "gui/widgets/textfield.h"

#include "net/inventoryhandler.h"

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
                        public gcn::SelectionListener,
                        public InventoryListener,
                        public Listener
{
    public:
        /**
         * Constructor.
         */
        InventoryWindow(Inventory *inventory);

        /**
         * Destructor.
         */
        ~InventoryWindow();

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

        /**
         * Handles the key presses.
         */
        void keyPressed(gcn::KeyEvent &event);

        /**
         * Handles the key releases.
         */
        void keyReleased(gcn::KeyEvent &event);

        /**
         * Updates labels to currently selected item.
         */
        void valueChanged(const gcn::SelectionEvent &event);

        /**
         * Sets whether the split button should be shown.
         */
        void setSplitAllowed(bool allowed);

        /**
         * Closes the Storage Window, as well as telling the server that the
         * window has been closed.
         */
        void close();

        /**
         * Updates the buttons.
         */
        void updateButtons();

        bool isInputFocused() const;

        static bool isAnyInputFocused();

        void slotsChanged(Inventory* inventory);

        bool isMainInventory() { return mInventory->isMainInventory(); }

        void event(Event::Channel channel, const Event &event);

    private:
        /**
         * Updates the weight bar.
         */
        void updateWeight();


        typedef std::list<InventoryWindow*> WindowList;
        static WindowList instances;

        Inventory *mInventory;
        ItemContainer *mItems;

        TextField *mFilterText;

        std::string mWeight, mSlots;

        gcn::Button *mUseButton, *mEquipButton, *mDropButton, *mSplitButton,
                    *mOutfitButton, *mStoreButton, *mRetrieveButton;

        gcn::Label *mWeightLabel, *mSlotsLabel, *mFilterLabel;

        ProgressBar *mWeightBar, *mSlotsBar;

        bool mSplit;
};

extern InventoryWindow *inventoryWindow;

#endif
