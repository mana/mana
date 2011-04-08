/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#ifndef SELL_H
#define SELL_H

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>
#include <guichan/selectionlistener.hpp>

#include <SDL_types.h>

class Item;
class ShopItems;
class ShopListBox;

/**
 * The sell dialog.
 *
 * \ingroup Interface
 */
class SellDialog : public Window, gcn::ActionListener, gcn::SelectionListener
{
    public:
        SellDialog(int npcId);

        virtual ~SellDialog();

        /**
         * Resets the dialog, clearing inventory.
         */
        void reset();

        /**
         * Adds an item to the inventory.
         */
        void addItem(const Item *item, int price);

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Updates labels according to selected item.
         *
         * @see SelectionListener::selectionChanged
         */
        void valueChanged(const gcn::SelectionEvent &event);

        /**
         * Gives Player's Money amount
         */
        void setMoney(int amount);

        /**
         * Sets the visibility of this window.
         */
        void setVisible(bool visible);

        /**
         * Closes all instances.
         */
        static void closeAll();

    private:
        typedef std::list<SellDialog*> DialogList;
        static DialogList instances;

        /**
         * Updates the state of buttons and labels.
         */
        void updateButtonsAndLabels();

        int mNpcId;

        gcn::Button *mSellButton;
        gcn::Button *mQuitButton;
        gcn::Button *mAddMaxButton;
        gcn::Button *mIncreaseButton;
        gcn::Button *mDecreaseButton;
        ShopListBox *mShopItemList;
        gcn::ScrollArea *mScrollArea;
        gcn::Label *mMoneyLabel;
        gcn::Label *mQuantityLabel;
        gcn::Slider *mSlider;

        ShopItems *mShopItems;
        int mPlayerMoney;

        int mMaxItems;
        int mAmountItems;
};

#endif
