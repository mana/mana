/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
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

#ifndef _TMW_SELL_H
#define _TMW_SELL_H

#include <guichan/actionlistener.hpp>

#include "window.h"
#include "selectionlistener.h"

#include "../guichanfwd.h"

class Item;
class ShopItems;
class ShopListBox;

/**
 * The sell dialog.
 *
 * \ingroup Interface
 */
class SellDialog : public Window, gcn::ActionListener, SelectionListener
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        SellDialog();

        /**
         * Destructor
         */
        virtual ~SellDialog();

        /**
         * Resets the dialog, clearing inventory.
         */
        void reset();

        /**
         * Adds an item to the inventory.
         */
        void addItem(Item *item, int price);

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string& eventId, gcn::Widget* widget);

        /**
         * Updates labels according to selected item.
         *
         * @see SelectionListener::selectionChanged
         */
        void selectionChanged(const SelectionEvent &event);

        /**
         * Gives Player's Money amount
         */
        void setMoney(int amount);

    private:
        gcn::Button *mSellButton;
        gcn::Button *mIncreaseButton;
        gcn::Button *mDecreaseButton;
        ShopListBox *mShopItemList;
        gcn::Label *mMoneyLabel;
        gcn::Label *mItemDescLabel;
        gcn::Label *mItemEffectLabel;
        gcn::Label *mQuantityLabel;
        gcn::Slider *mSlider;

        ShopItems *mShopItems;
        int mPlayerMoney;

        int mMaxItems;
        int mAmountItems;
};

#endif
