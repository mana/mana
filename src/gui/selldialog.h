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

#pragma once

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>
#include <guichan/selectionlistener.hpp>

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

        ~SellDialog() override;

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
        void action(const gcn::ActionEvent &event) override;

        /**
         * Updates labels according to selected item.
         *
         * @see SelectionListener::selectionChanged
         */
        void valueChanged(const gcn::SelectionEvent &event) override;

        /**
         * Allows for quick-selling by extending double-click events.
         */
        void mouseClicked(gcn::MouseEvent &mouseEvent) override;

        /**
         * Gives Player's Money amount
         */
        void setMoney(int amount);

        /**
         * Sets the visibility of this window.
         */
        void setVisible(bool visible) override;

        /**
         * Closes all instances.
         */
        static void closeAll();

    private:
        using DialogList = std::list<SellDialog *>;
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
        int mPlayerMoney = 0;

        int mMaxItems = 0;
        int mAmountItems = 0;
};
