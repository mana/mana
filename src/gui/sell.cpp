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

#include "sell.h"

#include <cassert>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "shoplistbox.h"
#include "scrollarea.h"
#include "shop.h"
#include "slider.h"

#include "../item.h"
#include "../npc.h"

#include "../resources/iteminfo.h"
#include "../resources/itemdb.h"

#include "../utils/tostring.h"

SellDialog::SellDialog():
    Window("Sell"),
    mMaxItems(0), mAmountItems(0)
{
    mShopItems = new ShopItems();

    mShopItemList = new ShopListBox(mShopItems, mShopItems);
    ScrollArea *scrollArea = new ScrollArea(mShopItemList);
    mSlider = new Slider(1.0);
    mQuantityLabel = new gcn::Label("0");
    mMoneyLabel = new gcn::Label("Money: 0 GP / Total: 0 GP");
    mItemDescLabel = new gcn::Label("Description:");
    mItemEffectLabel = new gcn::Label("Effect:");
    mIncreaseButton = new Button("+", "+", this);
    mDecreaseButton = new Button("-", "-", this);
    mSellButton = new Button("Sell", "sell", this);
    Button *quitButton = new Button("Quit", "quit", this);
    mSellButton->setEnabled(false);

    setContentSize(260, 210);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    scrollArea->setDimension(gcn::Rectangle(5, 5, 250, 110));
    mShopItemList->setDimension(gcn::Rectangle(5, 5, 238, 110));

    mSlider->setDimension(gcn::Rectangle(5, 120, 200, 10));
    mSlider->setEnabled(false);

    mQuantityLabel->setPosition(215, 120);

    mIncreaseButton->setPosition(40, 186);
    mIncreaseButton->setSize(20, 20);
    mIncreaseButton->setEnabled(false);

    mDecreaseButton->setPosition(10, 186);
    mDecreaseButton->setSize(20, 20);
    mDecreaseButton->setEnabled(false);

    mMoneyLabel->setPosition(5, 130);
    mItemEffectLabel->setDimension(gcn::Rectangle(5, 150, 240, 14));
    mItemDescLabel->setDimension(gcn::Rectangle(5, 169, 240, 14));

    mSellButton->setPosition(175, 186);
    mSellButton->setEnabled(false);

    quitButton->setPosition(208, 186);

    mShopItemList->setActionEventId("item");
    mSlider->setActionEventId("slider");

    mShopItemList->setPriceCheck(false);

    mShopItemList->addActionListener(this);
    mShopItemList->addSelectionListener(this);
    mSlider->addActionListener(this);

    add(scrollArea);
    add(mSlider);
    add(mQuantityLabel);
    add(mMoneyLabel);
    add(mItemEffectLabel);
    add(mItemDescLabel);
    add(mIncreaseButton);
    add(mDecreaseButton);
    add(mSellButton);
    add(quitButton);

    setLocationRelativeTo(getParent());
}

SellDialog::~SellDialog()
{
    delete mShopItems;
}

void SellDialog::reset()
{
    mShopItems->clear();
    mSlider->setValue(0.0);

    // Reset previous selected item to prevent failing asserts
    mShopItemList->setSelected(-1);

    updateButtonsAndLabels();
}

void SellDialog::addItem(int item, int amount, int price)
{
    mShopItems->addItem(item, amount, price);
    mShopItemList->adjustSize();
}

void SellDialog::action(const gcn::ActionEvent &event)
{
    int selectedItem = mShopItemList->getSelected();

    if (event.getId() == "item")
    {
        mAmountItems = 0;
        mSlider->setValue(0);
        updateButtonsAndLabels();
    }
    else if (event.getId() == "quit")
    {
        setVisible(false);
        current_npc = 0;
    }

    // The following actions require a valid item selection
    if (selectedItem == -1 ||
            selectedItem >= (int) mShopItems->getNumberOfElements())
    {
        return;
    }

    if (event.getId() == "slider")
    {
        mAmountItems = (int) (mSlider->getValue() * mMaxItems);
        updateButtonsAndLabels();
    }
    else if (event.getId() == "+" && mAmountItems < mMaxItems)
    {
        mAmountItems++;

        mSlider->setValue((double) mAmountItems /(double) mMaxItems);
        updateButtonsAndLabels();
    }
    else if (event.getId() == "-" && mAmountItems > 0)
    {
        mAmountItems--;

        mSlider->setValue((double) mAmountItems / (double) mMaxItems);
        updateButtonsAndLabels();
    }
    else if (event.getId() == "sell" && mAmountItems > 0
            && mAmountItems <= mMaxItems)
    {
        // Attempt sell
        // XXX Convert for new server
        /*
        MessageOut outMsg(CMSG_NPC_SELL_REQUEST);
        outMsg.writeShort(8);
        outMsg.writeShort(mShopItems->at(selectedItem).index);
        outMsg.writeShort(mAmountItems);
        */

        mMaxItems -= mAmountItems;
        mShopItems->getShop()->at(selectedItem).quantity = mMaxItems;
        mPlayerMoney += (mAmountItems * mShopItems->at(selectedItem).price);
        mAmountItems = 0;
        mSlider->setValue(0);

        if (!mMaxItems)
        {
            // All were sold
            mShopItemList->setSelected(-1);
            mShopItems->getShop()->erase(
                    mShopItems->getShop()->begin() + selectedItem);
        }
        else
        {
            // Update only when there are items left, the entry doesn't exist
            // otherwise and can't be updated
            updateButtonsAndLabels();
        }
    }
}

void SellDialog::selectionChanged(const SelectionEvent &event)
{
    // Reset amount of items and update labels
    mAmountItems = 0;
    mSlider->setValue(0);

    updateButtonsAndLabels();
}

void SellDialog::setMoney(int amount)
{
    mPlayerMoney = amount;
    mShopItemList->setPlayersMoney(amount);
}

void
SellDialog::updateButtonsAndLabels()
{
    int selectedItem = mShopItemList->getSelected();
    int income = 0;

    if (selectedItem > -1)
    {
        mMaxItems = mShopItems->at(selectedItem).quantity;
        if (mAmountItems > mMaxItems)
        {
            mAmountItems = mMaxItems;
        }

        income = mAmountItems * mShopItems->at(selectedItem).price;

        const ItemInfo &info = ItemDB::get(mShopItems->at(selectedItem).id);
        mItemDescLabel->setCaption("Description: " + info.getDescription());
        mItemEffectLabel->setCaption("Effect: " + info.getEffect());
    }
    else
    {
        mMaxItems = 0;
        mAmountItems = 0;
        mItemDescLabel->setCaption("Description:");
        mItemEffectLabel->setCaption("Effect:");
    }

    // Update Buttons and slider
    mSellButton->setEnabled(mAmountItems > 0);
    mDecreaseButton->setEnabled(mAmountItems > 0);
    mIncreaseButton->setEnabled(mAmountItems < mMaxItems);
    mSlider->setEnabled(selectedItem > -1);

    // Update the quantity and money labels
    mQuantityLabel->setCaption(
            toString(mAmountItems) + " / " + toString(mMaxItems));
    mQuantityLabel->adjustSize();
    mMoneyLabel->setCaption("Money: " + toString(income) + " GP / Total: "
                            + toString(mPlayerMoney + income) + " GP");
    mMoneyLabel->adjustSize();
}
