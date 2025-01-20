/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#ifndef SHOPLISTBOX_H
#define SHOPLISTBOX_H

#include "gui/widgets/listbox.h"

class ShopItems;
class ItemPopup;

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
        ShopListBox(gcn::ListModel *listModel);

        /**
         * Constructor with shopitems
         */
        ShopListBox(gcn::ListModel *listModel, ShopItems *shopListModel);

        ~ShopListBox() override;

        /**
         * Draws the list box.
         */
        void draw(gcn::Graphics *graphics) override;

        /**
         * Returns the height of a row.
         */
        unsigned int getRowHeight() const override { return mRowHeight; }

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

        /**
         ** Show ItemTooltip
         */
        void mouseMoved(gcn::MouseEvent &event) override;

        /**
         ** Hide ItemTooltip
         */
         void mouseExited(gcn::MouseEvent &event) override;

    private:
        int mPlayerMoney = 0;

        /**
         * Keeps another pointer to the same listModel, permitting to
         * use the ShopItems specific functions.
         */
        ShopItems *mShopItems;

        ItemPopup *mItemPopup;

        unsigned int mRowHeight; /**< Row Height */

        bool mPriceCheck;
};

#endif // SHOPLISTBOX_H
