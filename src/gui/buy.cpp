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
 */

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "buy.h"
#include "scrollarea.h"
#include "shop.h"
#include "shoplistbox.h"
#include "slider.h"

#include "../npc.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../utils/tostring.h"

BuyDialog::BuyDialog(Network *network):
    Window("Buy"), mNetwork(network),
    mMoney(0), mAmountItems(0), mMaxItems(0)
{
    setWindowName("Buy");
    setResizable(true);
    setMinWidth(260);
    setMinHeight(230);
    setDefaultSize(0, 0, 260, 230);

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

    mIncreaseButton->setSize(20, 20);
    mDecreaseButton->setSize(20, 20);
    mQuantityLabel->setWidth(60);

    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mIncreaseButton->setEnabled(false);
    mDecreaseButton->setEnabled(false);
    mBuyButton->setEnabled(false);
    mSlider->setEnabled(false);

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

    loadWindowState();
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

    // Reset previous selected items to prevent failing asserts
    mShopItemList->setSelected(-1);
    mSlider->setValue(0);

    setMoney(0);
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
        outMsg.writeInt16(mShopItems->at(selectedItem)->getId());

        // Update money and adjust the max number of items that can be bought
        mMaxItems -= mAmountItems;
        setMoney(mMoney -
                mAmountItems * mShopItems->at(selectedItem)->getPrice());

        // Reset selection
        mAmountItems = 1;
        mSlider->setValue(1);
        mSlider->gcn::Slider::setScale(1, mMaxItems);
    }
}

void BuyDialog::valueChanged(const gcn::SelectionEvent &event)
{
    // Reset amount of items and update labels
    mAmountItems = 1;
    mSlider->setValue(1);

    updateButtonsAndLabels();
    mSlider->gcn::Slider::setScale(1, mMaxItems);
}

void BuyDialog::widgetResized(const gcn::Event &event)
{
    Window::widgetResized(event);

    const gcn::Rectangle &area = getChildrenArea();
    const int width = area.width;
    const int height = area.height;

    mDecreaseButton->setPosition(8, height - 8 - mDecreaseButton->getHeight());
    mIncreaseButton->setPosition(
            mDecreaseButton->getX() + mDecreaseButton->getWidth() + 5,
            mDecreaseButton->getY());

    mQuitButton->setPosition(
            width - 8 - mQuitButton->getWidth(),
            height - 8 - mQuitButton->getHeight());
    mBuyButton->setPosition(
            mQuitButton->getX() - 5 - mBuyButton->getWidth(),
            mQuitButton->getY());

    mItemDescLabel->setDimension(gcn::Rectangle(8,
                mBuyButton->getY() - 5 - mItemDescLabel->getHeight(),
                width - 16,
                mItemDescLabel->getHeight()));
    mItemEffectLabel->setDimension(gcn::Rectangle(8,
                mItemDescLabel->getY() - 5 - mItemEffectLabel->getHeight(),
                width - 16,
                mItemEffectLabel->getHeight()));
    mMoneyLabel->setDimension(gcn::Rectangle(8,
                mItemEffectLabel->getY() - 5 - mMoneyLabel->getHeight(),
                width - 16,
                mMoneyLabel->getHeight()));

    mQuantityLabel->setPosition(
            width - mQuantityLabel->getWidth() - 8,
            mMoneyLabel->getY() - 5 - mQuantityLabel->getHeight());
    mSlider->setDimension(gcn::Rectangle(8,
                mQuantityLabel->getY(),
                mQuantityLabel->getX() - 8 - 8,
                10));

    mScrollArea->setDimension(gcn::Rectangle(8, 8, width - 16,
                mSlider->getY() - 5 - 8));
}

void
BuyDialog::updateButtonsAndLabels()
{
    const int selectedItem = mShopItemList->getSelected();
    int price = 0;

    if (selectedItem > -1)
    {
        const ItemInfo &info = mShopItems->at(selectedItem)->getInfo();

        mItemDescLabel->setCaption("Description: " + info.getDescription());
        mItemEffectLabel->setCaption("Effect: " + info.getEffect());

        int itemPrice = mShopItems->at(selectedItem)->getPrice();

        // Calculate how many the player can afford
        mMaxItems = mMoney / itemPrice;
        if (mAmountItems > mMaxItems)
        {
            mAmountItems = mMaxItems;
        }

        // Calculate price of pending purchase
        price = mAmountItems * itemPrice;
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
    mMoneyLabel->setCaption("Price: " + toString(price)  + " GP / "
                             + toString(mMoney - price) + " GP" );
}
