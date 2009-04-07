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

#ifndef SHOPLISTBOX_H
#define SHOPLISTBOX_H

#include "gui/widgets/listbox.h"

class ShopItems;

/**
 * A list box, meant to be used inside a scroll area. Same as the Guichan list
 * box except this one doesn't have a background, instead completely relying
 * on the scroll area. It also adds selection listener functionality.
 *
 * \ingroup GUI
 */
class ShopListBox : public ListBox
{
    public:
        /**
         * Constructor.
         */
        ShopListBox(gcn::ListModel *listModel);

        /**
         * Constructor with shopitems
         */
        ShopListBox(gcn::ListModel *listModel, ShopItems *shopListModel);

        /**
         * Draws the list box.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Returns the height of a row.
         */
        unsigned int getRowHeight() const { return mRowHeight; }

        /**
         * gives information about the current player's money
         */
        void setPlayersMoney(int money);

        /**
         * Adjust List draw size
         */
        void adjustSize();

        /**
         * Set on/off the disabling of too expensive items.
         * (Good for selling mode.)
         */
        void setPriceCheck(bool check);

    private:
        int mPlayerMoney;

        /**
         * Keeps another pointer to the same listModel, permitting to
         * use the ShopItems specific functions.
         */
        ShopItems *mShopItems;

        unsigned int mRowHeight; /**< Row Height */

        static float mAlpha;

        bool mPriceCheck;
};

#endif // SHOPLISTBOX_H
