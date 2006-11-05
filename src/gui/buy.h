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

#ifndef _TMW_BUY_H
#define _TMW_BUY_H

#include <guichan/actionlistener.hpp>

#include "window.h"
#include "selectionlistener.h"
#include "shoplistbox.h"

#include "../guichanfwd.h"

class Network;
class ShopItems;
class ListBox;

/**
 * The buy dialog.
 *
 * \ingroup Interface
 */
class BuyDialog : public Window, public gcn::ActionListener, SelectionListener
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        BuyDialog(Network *network);

        /**
         * Destructor
         */
        ~BuyDialog();

        /**
         * Resets the dialog, clearing shop inventory.
         */
        void reset();

        /**
         * Sets the amount of available money.
         */
        void setMoney(int amount);

        /**
         * Adds an item to the shop inventory.
         */
        void addItem(short id, int price);

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string& eventId, gcn::Widget* widget);

        /**
         * Returns the number of items in the shop inventory.
         */
        int getNumberOfElements();

        /**
         * Updates the labels according to the selected item.
         *
         * @see SelectionListener::selectionChanged
         */
        void selectionChanged(const SelectionEvent &event);

        /**
         * Returns the name of item number i in the shop inventory.
         */
        std::string getElementAt(int i);

    private:
        Network *mNetwork;
        gcn::Button *mBuyButton;
        gcn::Button *mQuitButton;
        gcn::Button *mIncreaseButton;
        gcn::Button *mDecreaseButton;
        ShopListBox *mShopItemList;
        gcn::ScrollArea *mScrollArea;
        gcn::Label *mItemDescLabel;
        gcn::Label *mItemEffectLabel;
        gcn::Label *mMoneyLabel;
        gcn::Label *mQuantityLabel;
        gcn::Slider *mSlider;

        ShopItems *mShopItems;

        int mMoney;
        int mAmountItems;
        int mMaxItems;
};

#endif
