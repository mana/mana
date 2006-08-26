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
#include "listbox.h"
#include "scrollarea.h"
#include "shop.h"
#include "slider.h"

#include "../npc.h"

#include "../resources/iteminfo.h"
#include "../resources/itemmanager.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../utils/tostring.h"


BuyDialog::BuyDialog():
    Window("Buy"),
    mMoney(0), mAmountItems(0), mMaxItems(0)
{
    mShopItems = new ShopItems;

    mItemList = new ListBox(mShopItems);
    mScrollArea = new ScrollArea(mItemList);
    mSlider = new Slider(1.0);
    mQuantityLabel = new gcn::Label("0");
    mMoneyLabel = new gcn::Label("Price: 0 GP");
    mIncreaseButton = new Button("+", "+", this);
    mDecreaseButton = new Button("-", "-", this);
    mBuyButton = new Button("Buy", "buy", this);
    mQuitButton = new Button("Quit", "quit", this);
    mItemDescLabel = new gcn::Label("Description:");
    mItemEffectLabel = new gcn::Label("Effect:");

    setContentSize(260, 210);
    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mScrollArea->setDimension(gcn::Rectangle(5, 5, 250, 110));
    mItemList->setDimension(gcn::Rectangle(5, 5, 238, 110));

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

    mItemList->setEventId("item");
    mSlider->setEventId("slider");

    mItemList->addActionListener(this);
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
}

void BuyDialog::reset()
{
    mShopItems->clear();
    mMoney = 0;
    mSlider->setValue(0.0);
    mAmountItems = 0;

    // Reset Previous Selected Items to prevent failing asserts
    mItemList->setSelected(-1);
    mIncreaseButton->setEnabled(false);
    mDecreaseButton->setEnabled(false);
    mQuantityLabel->setCaption("0");
    mQuantityLabel->adjustSize();
    mMoneyLabel->setCaption("Price: 0");
    mMoneyLabel->adjustSize();
    mItemDescLabel->setCaption("");
    mItemEffectLabel->setCaption("");
}

void BuyDialog::addItem(short id, int price)
{
    ITEM_SHOP item_shop;

    item_shop.name = itemDb->getItemInfo(id)->getName() + " " + toString(price) + " GP";
    item_shop.price = price;
    item_shop.id = id;

    mShopItems->push_back(item_shop);
    mItemList->adjustSize();
}

void BuyDialog::action(const std::string &eventId, gcn::Widget *widget)
{
    int selectedItem = mItemList->getSelected();

    if (eventId == "item") {
        // Reset amount of items and update labels
        mAmountItems = 0;
        mSlider->setValue(0);
        mQuantityLabel->setCaption("0");
        mQuantityLabel->adjustSize();
        mMoneyLabel->setCaption("Price : 0 GP");
        mMoneyLabel->adjustSize();

        // Disable buttons for buying and decreasing
        mBuyButton->setEnabled(false);
        mDecreaseButton->setEnabled(false);

        // If no item was selected, none can be bought, otherwise
        // calculate how many the player can afford
        mMaxItems = (mItemList->getSelected() == -1) ? 0 :
            mMoney / mShopItems->at(selectedItem).price;

        // When at least one item can be bought, enable the slider and the
        // increase button
        mIncreaseButton->setEnabled(mMaxItems > 0);
        mSlider->setEnabled(mMaxItems > 0);
    }
    else if (eventId == "quit") {
        setVisible(false);
        current_npc = 0;
    }

    // The following actions require a valid selection
    if (selectedItem < 0 || selectedItem >= int(mShopItems->size())) {
        return;
    }

    bool updateButtonsAndLabels = false;

    if (eventId == "slider") {
        mAmountItems = (int)(mSlider->getValue() * mMaxItems);
        updateButtonsAndLabels = true;
    }
    else if (eventId == "+") {
        if (mAmountItems < mMaxItems) {
            mAmountItems++;
        } else {
            mAmountItems = mMaxItems;
        }

        mSlider->setValue(double(mAmountItems)/double(mMaxItems));
        updateButtonsAndLabels = true;
    }
    else if (eventId == "-") {
        if (mAmountItems > 0) {
            mAmountItems--;
        } else {
            mAmountItems = 0;
        }

        mSlider->setValue(double(mAmountItems)/double(mMaxItems));
        updateButtonsAndLabels = true;
    }
    // TODO Actually we'd have a bug elsewhere if this check for the number
    // of items to be bought ever fails, Bertram removed the assertions, is
    // there a better way to ensure this fails in an _obivous_ way in C++?
    else if (eventId == "buy" && (mAmountItems > 0 &&
                mAmountItems <= mMaxItems))
    {
        MessageOut outMsg(CMSG_NPC_BUY_REQUEST);
        outMsg.writeShort(8);
        outMsg.writeShort(mAmountItems);
        outMsg.writeShort(mShopItems->at(selectedItem).id);

        // update money !
        mMoney -= mAmountItems * mShopItems->at(selectedItem).price;
        // Update number of items that can be bought at max
        mMaxItems -= mAmountItems;

        if (!mMaxItems) {
            mSlider->setEnabled(false);
        }

        // Reset selection
        mAmountItems = 0;
        mSlider->setValue(0);

        updateButtonsAndLabels = true;
    }

    // If anything has changed, we have to update the buttons and labels
    if (updateButtonsAndLabels) {
        // Update buttons
        mIncreaseButton->setEnabled(mAmountItems < mMaxItems);
        mDecreaseButton->setEnabled(mAmountItems > 0);
        mBuyButton->setEnabled(mAmountItems > 0);

        // Update labels
        mQuantityLabel->setCaption(toString(mAmountItems));
        mQuantityLabel->adjustSize();

        int price = mAmountItems * mShopItems->at(selectedItem).price;
        mMoneyLabel->setCaption("Price : " + toString(price)  + " GP");
        mMoneyLabel->adjustSize();
    }
}

void BuyDialog::mouseClick(int x, int y, int button, int count)
{
    Window::mouseClick(x, y, button, count);

    int selectedItem = mItemList->getSelected();
    if (selectedItem > -1)
    {
        int itemId = mShopItems->at(selectedItem).id;
        ItemInfo *itemInfo = itemDb->getItemInfo(itemId);

        mItemDescLabel->setCaption("Description: " +
                itemInfo->getDescription());
        mItemEffectLabel->setCaption("Effect: " +
                itemInfo->getEffect());
    }
}
