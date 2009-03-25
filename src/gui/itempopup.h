/*
 *  The Mana World
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008  The Mana World Development Team
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

#ifndef ITEMPOPUP_H
#define ITEMPOPUP_H

#include "popup.h"

class ItemInfo;
class ScrollArea;
class TextBox;

class ItemPopup : public Popup
{
    public:
        /**
         * Constructor. Initializes the item popup.
         */
        ItemPopup();

        /**
         * Destructor. Cleans up the item popup on deletion.
         */
        ~ItemPopup();

        /**
         * Sets the info to be displayed given a particular item.
         */
        void setItem(const ItemInfo &item);

        /**
         * Gets the number of rows that the item popup currently has.
         */
        unsigned int getNumRows();

        /**
         * Gets the name of the currently stored item in this popup.
         */
        std::string getItemName();

        /**
         * Updates the colors used within the item popup.
         */
        void updateColors();

        /**
         * Sets the location to display the item popup.
         */
        void view(int x, int y);

    private:
        gcn::Label *mItemName;
        TextBox *mItemDesc;
        TextBox *mItemEffect;
        TextBox *mItemWeight;
        std::string mItemType;
        ScrollArea *mItemDescScroll;
        ScrollArea *mItemEffectScroll;
        ScrollArea *mItemWeightScroll;

        static gcn::Color getColor(const std::string &type);
};

#endif // ITEMPOPUP_H
