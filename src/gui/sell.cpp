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
#include "../resources/itemmanager.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../utils/tostring.h"

SellDialog::SellDialog(Network *network):
    Window("Sell"),
    mNetwork(network),
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

    mShopItemList->setEventId("item");
    mSlider->setEventId("mSlider");

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
    mAmountItems = 0;

    mQuantityLabel->setCaption("0");
    mQuantityLabel->adjustSize();
    mMoneyLabel->setCaption("Money: 0 GP / Total: "
            + toString(mPlayerMoney) + " GP");
    mMoneyLabel->adjustSize();
    mItemDescLabel->setCaption("");
    mItemEffectLabel->setCaption("");

    // Reset Previous Selected Items to prevent failing asserts
    mShopItemList->setSelected(-1);
    mIncreaseButton->setEnabled(false);
    mDecreaseButton->setEnabled(false);
}

void SellDialog::addItem(Item *item, int price)
{
    if (!item)
        return;

    ITEM_SHOP item_shop;

    item_shop.name = item->getInfo().getName() + " " + toString(price) + " GP";
    item_shop.price = price;
    item_shop.index = item->getInvIndex();
    item_shop.id = item->getId();
    item_shop.quantity = item->getQuantity();
    item_shop.image = item->getInfo().getImage();

    mShopItems->push_back(item_shop);
    mShopItemList->adjustSize();
}

void SellDialog::action(const std::string& eventId, gcn::Widget* widget)
{
    int selectedItem = mShopItemList->getSelected();

    if (eventId == "item")
    {
        mAmountItems = 0;
        mSlider->setValue(0);
        mDecreaseButton->setEnabled(false);
        mSellButton->setEnabled(false);

        mQuantityLabel->setCaption("0");
        mQuantityLabel->adjustSize();
        mMoneyLabel->setCaption("Money: 0 GP / Total: "
            + toString(mPlayerMoney) + " GP");
        mMoneyLabel->adjustSize();

        if (selectedItem > -1) {
            mSlider->setEnabled(true);
            mIncreaseButton->setEnabled(true);
            mMaxItems = mShopItems->at(selectedItem).quantity;
        } else {
            mSlider->setEnabled(false);
            mIncreaseButton->setEnabled(false);
        }
    }
    else if (eventId == "quit")
    {
        setVisible(false);
        current_npc = 0;
    }

    // The following actions require a valid item selection
    if (selectedItem == -1 || selectedItem >= int(mShopItems->getNumberOfElements())) {
        return;
    }

    bool updateButtonsAndLabels = false;

    if (eventId == "mSlider")
    {
        mAmountItems = (int)(mSlider->getValue() * mMaxItems);

        updateButtonsAndLabels = true;
    }
    else if (eventId == "+")
    {
        assert(mAmountItems < mMaxItems);
        mAmountItems++;
        mSlider->setValue(double(mAmountItems)/double(mMaxItems));

        updateButtonsAndLabels = true;
    }
    else if (eventId == "-")
    {
        assert(mAmountItems > 0);
        mAmountItems--;

        mSlider->setValue(double(mAmountItems)/double(mMaxItems));

        updateButtonsAndLabels = true;
    }
    else if (eventId == "sell")
    {
        // Attempt sell
        assert(mAmountItems > 0 && mAmountItems <= mMaxItems);

        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(CMSG_NPC_SELL_REQUEST);
        outMsg.writeInt16(8);
        outMsg.writeInt16(mShopItems->at(selectedItem).index);
        outMsg.writeInt16(mAmountItems);

        mMaxItems -= mAmountItems;
        mAmountItems = 0;
        mSlider->setValue(0);
        mSlider->setEnabled(mMaxItems != 0);

        // All were sold
        if (!mMaxItems) {
            mShopItemList->setSelected(-1);
            mShopItems->getShop().erase(mShopItems->getShop().begin() + selectedItem);
        }

        // Update only when there are items left, the entry doesn't exist
        // otherwise and can't be updated
        updateButtonsAndLabels = bool(mMaxItems);
    }

    // If anything changed, we need to update the buttons and labels
    if (updateButtonsAndLabels)
    {
        // Update labels
        mQuantityLabel->setCaption(toString(mAmountItems));
        mQuantityLabel->adjustSize();

        int price = mAmountItems * mShopItems->at(selectedItem).price;
        mMoneyLabel->setCaption("Money: " + toString(price) + " GP / Total: "
            + toString(price + mPlayerMoney) + " GP");
        mMoneyLabel->adjustSize();

        // Update Buttons
        mSellButton->setEnabled(mAmountItems > 0);
        mDecreaseButton->setEnabled(mAmountItems > 0);
        mIncreaseButton->setEnabled(mAmountItems < mMaxItems);
    }
}

void SellDialog::selectionChanged(const SelectionEvent &event)
{
    int selectedItem = mShopItemList->getSelected();

    if (selectedItem > -1)
    {
        const ItemInfo &info =
            itemDb->getItemInfo(mShopItems->at(selectedItem).id);

        mItemDescLabel->setCaption("Description: " + info.getDescription());
        mItemEffectLabel->setCaption("Effect: " + info.getEffect());
    }
    else
    {
        mItemDescLabel->setCaption("Description");
        mItemEffectLabel->setCaption("Effect");
    }
}

void SellDialog::setMoney(int amount)
{
    mPlayerMoney = amount;
    mShopItemList->setPlayersMoney(amount);
}
