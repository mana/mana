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
 */

#ifndef _TMW_SMILEYSHORTCUT_H__
#define _TMW_SMILEYSHORTCUT_H__

#define SHORTCUT_SMILEYS 12

class Item;

/**
 * The class which keeps track of the item shortcuts.
 */
class SmileyShortcut
{
    public:
        /**
         * Constructor.
         */
        SmileyShortcut();

        /**
         * Destructor.
         */
        ~SmileyShortcut();

        /**
         * Load the configuration information.
         */
        void load();

        /**
         * Returns the shortcut smiley ID specified by the index.
         *
         * @param index Index of the shortcut smiley.
         */
        int getSmiley(int index) const
        { return mSmileys[index]; }

        /**
         * Returns the amount of shortcut smileys.
         */
        int getSmileyCount() const
        { return SHORTCUT_SMILEYS; }

        /**
         * Returns the item ID that is currently selected.
         */
        int getSmileySelected() const
        { return mSmileySelected; }

        /**
         * Adds the selected item ID to the items specified by the index.
         *
         * @param index Index of the items.
         */
        void setSmiley(int index)
        { mSmileys[index] = mSmileySelected; }

        /**
         * Adds a smiley to the smileys store specified by the index.
         *
         * @param index Index of the smiley.
         * @param smileyId ID of the smiley.
         */
        void setSmileys(int index, int smileyId)
        { mSmileys[index] = smileyId; }

        /**
         * Set the smiley that is selected.
         *
         * @param smileyId The ID of the smiley that is to be assigned.
         */
        void setSmileySelected(int smileyId)
        { mSmileySelected = smileyId; }

        /**
         * A flag to check if the smiley is selected.
         */
        bool isSmileySelected()
        { return mSmileySelected; }

        /**
         * Remove a smiley from the shortcut.
         */
        void removeSmiley(int index)
        { mSmileys[index] = 0; }

        /**
         * Try to use the smiley specified by the index.
         *
         * @param index Index of the smiley shortcut.
         */
        void useSmiley(int index);

    private:
        /**
         * Save the configuration information.
         */
        void save();

        int mSmileys[SHORTCUT_SMILEYS]; /**< The smiley stored. */
        int mSmileySelected;            /**< The smiley held by cursor. */

};

extern SmileyShortcut *smileyShortcut;

#endif
