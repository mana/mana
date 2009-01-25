/*
 *  The Mana World
 *  Copyright (C) 2007  The Mana World Development Team
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

#ifndef ITEMSHORTCUT_H
#define ITEMSHORTCUT_H

#define SHORTCUT_ITEMS 12

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
         * Returns the shortcut item ID specified by the index.
         *
         * @param index Index of the shortcut item.
         */
        int getItem(int index) const
        { return mItems[index]; }

        /**
         * Returns the amount of shortcut items.
         */
        int getItemCount() const
        { return SHORTCUT_ITEMS; }

        /**
         * Returns the item ID that is currently selected.
         */
        int getItemSelected() const
        { return mItemSelected; }

        /**
         * Adds the selected item ID to the items specified by the index.
         *
         * @param index Index of the items.
         */
        void setItem(int index)
        { mItems[index] = mItemSelected; }

        /**
         * Adds an item to the items store specified by the index.
         *
         * @param index Index of the item.
         * @param itemId ID of the item.
         */
        void setItems(int index, int itemId)
        { mItems[index] = itemId; }

        /**
         * Set the item that is selected.
         *
         * @param itemId The ID of the item that is to be assigned.
         */
        void setItemSelected(int itemId)
        { mItemSelected = itemId; }

        /**
         * A flag to check if the item is selected.
         */
        bool isItemSelected()
        { return mItemSelected > -1; }

        /**
         * Remove a item from the shortcut.
         */
        void removeItem(int index)
        { mItems[index] = -1; }

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

        int mItems[SHORTCUT_ITEMS];     /**< The items stored. */
        int mItemSelected;              /**< The item held by cursor. */

};

extern ItemShortcut *itemShortcut;

#endif
