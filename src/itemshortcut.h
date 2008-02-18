/*
 *  The Mana World
 *  Copyright 2007 The Mana World Development Team
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

#ifndef _TMW_ITEMSHORTCUT_H__
#define _TMW_ITEMSHORTCUT_H__

#define SHORTCUT_ITEMS 10

class Item;

/**
 * The class which keeps track of the item shortcuts.
 */
class ItemShortcut
{
    public:
        /**
         * Constructor.
         */
        ItemShortcut();

        /**
         * Destructor.
         */
        ~ItemShortcut();

        /**
         * Load the configuration information.
         */
        void load();

        /**
         * Returns the shortcut item specified by the index.
         *
         * @param index Index of the shortcut item.
         */
        Item* getItem(int index) const
        { return mItems[index]; }

        /**
         * Returns the amount of shortcut items.
         */
        int getItemCount() const
        { return SHORTCUT_ITEMS; }

        /**
         * Returns the item that is currently selected.
         */
        Item* getItemSelected() const
        { return mItemSelected; }

        /**
         * Adds the selected item to the items specified by the index.
         *
         * @param index Index of the items.
         */
        void setItem(int index)
        { mItems[index] = mItemSelected; }

        /**
         * Adds an item to the items store specified by the index.
         *
         * @param index Index of the items.
         * @param item Item to store.
         */
        void setItems(int index, Item *item)
        { mItems[index] = item; }

        /**
         * Set the item that is selected.
         *
         * @param item The item that is to be assigned.
         */
        void setItemSelected(Item* item)
        { mItemSelected = item; }

        /**
         * A flag to check if the item is selected.
         */
        bool isItemSelected()
        { return mItemSelected; }

        /**
         * Remove a item from the shortcut.
         */
        void removeItem(int index)
        { mItems[index] = 0; }

        /**
         * Try to use the item specified by the index.
         *
         * @param index Index of the item shortcut.
         */
        void useItem(int index);

    private:
        /**
         * Save the configuration information.
         */
        void save();

        Item* mItems[SHORTCUT_ITEMS];     /**< The items stored. */
        Item* mItemSelected;              /**< The item held by cursor. */

};

extern ItemShortcut *itemShortcut;

#endif
