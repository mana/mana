/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef STORAGEWINDOW_H
#define STORAGEWINDOW_H

#include "inventory.h"

#include "gui/widgets/window.h"

#include "net/inventoryhandler.h"
#include "net/net.h"

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
                                       gcn::SelectionListener,
                                       public InventoryListener
{
    public:
        /**
         * Constructor.
         */
        StorageWindow(Inventory *inventory);

        /**
         * Destructor.
         */
        ~StorageWindow();

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
         * Closes the Storage Window, as well as telling the server that the
         * window has been closed.
         */
        void close();

        void slotsChanged(Inventory* inventory);

        /**
         * Add the specified ammount of the specified item to storage
         */
        static void addStore(Item* item, int amount);

        /**
         * Remove the specified ammount of the specified item from storage
         */
        static void removeStore(Item* item, int amount);

        /**
         * Returns true if any instances exist.
         */
        static bool isActive() { return instances.size() > 0; }

    private:
        typedef std::list<StorageWindow*> WindowList;
        static WindowList instances;

        Inventory *mInventory;
        ItemContainer *mItems;

        int mSlots;
        int mUsedSlots;
        gcn::Button *mStoreButton, *mRetrieveButton, *mCloseButton;

        gcn::Label *mSlotsLabel;

        ProgressBar *mSlotsBar;

        bool mItemDesc;
};

#endif // STORAGEWINDOW_H
