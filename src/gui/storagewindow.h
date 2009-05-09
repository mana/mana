/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef STORAGEWINDOW_H
#define STORAGEWINDOW_H

#include "gui/widgets/window.h"

#include "inventory.h"

#include <guichan/actionlistener.hpp>
#include <guichan/selectionlistener.hpp>

class Item;
class ItemContainer;
class ProgressBar;
class TextBox;

/**
 * Storage dialog.
 *
 * \ingroup Interface
 */
class StorageWindow : public Window, gcn::ActionListener,
                                       gcn::SelectionListener
{
    public:
        /**
         * Constructor.
         */
        StorageWindow(int invSize = (STORAGE_SIZE - 1));

        /**
         * Destructor.
         */
        ~StorageWindow();

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

        /**
         * Add the specified ammount of the specified item to storage
         */
        void addStore(Item* item, int amount);

        /**
         * Remove the specified ammount of the specified item from storage
         */
        void removeStore(Item* item, int amount);

        /**
         * Closes the Storage Window, as well as telling the server that the
         * window has been closed.
         */
        void close();

    private:
        ItemContainer *mItems;

        int mSlots;
        int mUsedSlots;
        gcn::Button *mStoreButton, *mRetrieveButton, *mCloseButton;

        gcn::Label *mSlotsLabel;

        ProgressBar *mSlotsBar;

        int mMaxSlots;

        bool mItemDesc;
};

extern StorageWindow *storageWindow;

#endif // STORAGEWINDOW_H
