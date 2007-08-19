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

#include "buy.h"

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "scrollarea.h"
#include "shop.h"
#include "slider.h"

#include "../npc.h"

#include "../resources/itemdb.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../utils/tostring.h"

BuyDialog::BuyDialog(Network *network):
    Window("Buy"), mNetwork(network),
    mMoney(0), mAmountItems(0), mMaxItems(0)
{
    mShopItems = new ShopItems;

    mShopItemList = new ShopListBox(mShopItems, mShopItems);
    mScrollArea = new ScrollArea(mShopItemList);
    mSlider = new Slider(1.0);
    mQuantityLabel = new gcn::Label("0");
    mMoneyLabel = new gcn::Label("Price : 0 GP / 0 GP");
    mIncreaseButton = new Button("+", "+", this);
    mDecreaseButton = new Button("-", "-", this);
    mBuyButton = new Button("Buy", "buy", this);
    mQuitButton = new Button("Quit", "quit", this);
    mItemDescLabel = new gcn::Label("Description:");
    mItemEffectLabel = new gcn::Label("Effect:");

    setContentSize(260, 210);
    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mScrollArea->setDimension(gcn::Rectangle(5, 5, 250, 110));
    mShopItemList->setDimension(gcn::Rectangle(5, 5, 238, 110));

    mSlider->setDimension(gcn::Rectangle(5, 120, 200, 10));
    mSlider->setEnabled(false);

    mQuantityLabel->setPosition(215, 120);
    mMoneyLabel->setPosition(5, 130);

    mIncreaseButton->setPosition(40, 186);
    mIncreaseButton->setSize(20, 20);
    mIncreaseButton->setEnabled(false);

    mDecreaseButton->setPosition(10, 186);
    mDecreaseButton->setSize(20, 20);
    mDecreaseButton->setEnabled(false);

    mBuyButton->setPosition(180, 186);
    mBuyButton->setEnabled(false);

    mQuitButton->setPosition(212, 186);

    mItemEffectLabel->setDimension(gcn::Rectangle(5, 150, 240, 14));
    mItemDescLabel->setDimension(gcn::Rectangle(5, 169, 240, 14));

    mShopItemList->setActionEventId("item");
    mSlider->setActionEventId("slider");

    mShopItemList->addSelectionListener(this);
    mSlider->addActionListener(this);

    add(mScrollArea);
    add(mSlider);
    add(mQuantityLabel);
    add(mBuyButton);
    add(mQuitButton);
    add(mIncreaseButton);
    add(mDecreaseButton);
    add(mMoneyLabel);
    add(mItemDescLabel);
    add(mItemEffectLabel);

    setLocationRelativeTo(getParent());
}

BuyDialog::~BuyDialog()
{
    delete mShopItems;
}

void BuyDialog::setMoney(int amount)
{
    mMoney = amount;
    mShopItemList->setPlayersMoney(amount);

    updateButtonsAndLabels();
}

void BuyDialog::reset()
{
    mShopItems->clear();
    mShopItemList->adjustSize();
    mMoney = 0;
    mSlider->setValue(0);

    // Reset Previous Selected Items to prevent failing asserts
    mShopItemList->setSelected(-1);

    updateButtonsAndLabels();
}

void BuyDialog::addItem(short id, int price)
{
    mShopItems->addItem(id, price);
    mShopItemList->adjustSize();
}

void BuyDialog::action(const gcn::ActionEvent &event)
{
    int selectedItem = mShopItemList->getSelected();

    if (event.getId() == "quit")
    {
        setVisible(false);
        current_npc = 0;
        return;
    }

    // The following actions require a valid selection
    if (selectedItem < 0 ||
            selectedItem >= (int) mShopItems->getNumberOfElements())
    {
        return;
    }

    if (event.getId() == "slider")
    {
        mAmountItems = (int) mSlider->getValue();
        updateButtonsAndLabels();
    }
    else if (event.getId() == "+" && mAmountItems < mMaxItems)
    {
        mAmountItems++;
        mSlider->setValue(mAmountItems);
        updateButtonsAndLabels();
    }
    else if (event.getId() == "-" && mAmountItems > 1)
    {
        mAmountItems--;
        mSlider->setValue(mAmountItems);
        updateButtonsAndLabels();
    }
    // TODO: Actually we'd have a bug elsewhere if this check for the number
    // of items to be bought ever fails, Bertram removed the assertions, is
    // there a better way to ensure this fails in an _obivous_ way in C++?
    else if (event.getId() == "buy" && mAmountItems > 0 &&
                mAmountItems <= mMaxItems)
    {
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(CMSG_NPC_BUY_REQUEST);
        outMsg.writeInt16(8);
        outMsg.writeInt16(mAmountItems);
        outMsg.writeInt16(mShopItems->at(selectedItem).id);

        // Update money and adjust the max number of items that can be bought
        mMoney -= mAmountItems * mShopItems->at(selectedItem).price;
        mMaxItems -= mAmountItems;

        // Reset selection
        mAmountItems = 1;
        mSlider->setValue(1);
        mSlider->gcn::Slider::setScale(1, mMaxItems);

        updateButtonsAndLabels();
    }
}

void BuyDialog::selectionChanged(const SelectionEvent &event)
{

    // Reset amount of items and update labels
    mAmountItems = 1;
    mSlider->setValue(1);

    updateButtonsAndLabels();
    mSlider->gcn::Slider::setScale(1, mMaxItems);
}

void
BuyDialog::updateButtonsAndLabels()
{
    int selectedItem = mShopItemList->getSelected();
    int price = 0;

    if (selectedItem > -1)
    {
        const ItemInfo &info = ItemDB::get(mShopItems->at(selectedItem).id);

        mItemDescLabel->setCaption("Description: " + info.getDescription());
        mItemEffectLabel->setCaption("Effect: " + info.getEffect());

        // Calculate how many the player can afford
        mMaxItems = mMoney / mShopItems->at(selectedItem).price;
        if (mAmountItems > mMaxItems)
        {
            mAmountItems = mMaxItems;
        }

        // Calculate price of pending purchase
        price = mAmountItems * mShopItems->at(selectedItem).price;
    }
    else
    {
        mItemDescLabel->setCaption("Description:");
        mItemEffectLabel->setCaption("Effect:");
        mMaxItems = 0;
        mAmountItems = 0;
    }

    // Enable or disable buttons and slider
    mIncreaseButton->setEnabled(mAmountItems < mMaxItems);
    mDecreaseButton->setEnabled(mAmountItems > 1);
    mBuyButton->setEnabled(mAmountItems > 0);
    mSlider->setEnabled(mMaxItems > 1);

    // Update quantity and money labels
    mQuantityLabel->setCaption(
        toString(mAmountItems) + " / " + toString(mMaxItems));
    mQuantityLabel->adjustSize();
    mMoneyLabel->setCaption("Price: " + toString(price)  + " GP / "
                             + toString(mMoney - price) + " GP" );
    mMoneyLabel->adjustSize();
}
