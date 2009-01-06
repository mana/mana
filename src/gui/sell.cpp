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

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"

SellDialog::SellDialog(Network *network):
    Window(_("Sell")),
    mNetwork(network),
    mMaxItems(0), mAmountItems(0)
{
    setWindowName("Sell");
    setResizable(true);
    setMinWidth(260);
    setMinHeight(230);
    setDefaultSize(0, 0, 260, 230);

    mShopItems = new ShopItems();

    mShopItemList = new ShopListBox(mShopItems, mShopItems);
    mScrollArea = new ScrollArea(mShopItemList);
    mSlider = new Slider(1.0);
    mQuantityLabel = new gcn::Label("0");
    mMoneyLabel = new gcn::Label(
        strprintf(_("Price: %d GP / Total: %d GP"), 0, 0));
    mIncreaseButton = new Button("+", "+", this);
    mDecreaseButton = new Button("-", "-", this);
    mSellButton = new Button(_("Sell"), "sell", this);
    mQuitButton = new Button(_("Quit"), "quit", this);
    mItemDescLabel = new gcn::Label(strprintf(_("Description: %s"), ""));
    mItemEffectLabel = new gcn::Label(strprintf(_("Effect: %s"), ""));

    mIncreaseButton->setSize(20, 20);
    mDecreaseButton->setSize(20, 20);
    mQuantityLabel->setWidth(60);

    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mIncreaseButton->setEnabled(false);
    mDecreaseButton->setEnabled(false);
    mSellButton->setEnabled(false);
    mSlider->setEnabled(false);

    mShopItemList->setPriceCheck(false);
    mShopItemList->setActionEventId("item");
    mSlider->setActionEventId("slider");

    mShopItemList->addActionListener(this);
    mShopItemList->addSelectionListener(this);
    mSlider->addActionListener(this);

    add(mScrollArea);
    add(mSlider);
    add(mQuantityLabel);
    add(mMoneyLabel);
    add(mItemEffectLabel);
    add(mItemDescLabel);
    add(mIncreaseButton);
    add(mDecreaseButton);
    add(mSellButton);
    add(mQuitButton);

    loadWindowState();
    setLocationRelativeTo(getParent());
}

SellDialog::~SellDialog()
{
    delete mShopItems;
}

void SellDialog::reset()
{
    mShopItems->clear();
    mSlider->setValue(0);

    // Reset previous selected item to prevent failing asserts
    mShopItemList->setSelected(-1);

    updateButtonsAndLabels();
}

void SellDialog::addItem(const Item *item, int price)
{
    if (!item)
        return;

    mShopItems->addItem(
            item->getInvIndex(), item->getId(),
            item->getQuantity(), price);

    mShopItemList->adjustSize();
}

void SellDialog::action(const gcn::ActionEvent &event)
{
    int selectedItem = mShopItemList->getSelected();

    if (event.getId() == "quit")
    {
        setVisible(false);
        current_npc = 0;
        return;
    }

    // The following actions require a valid item selection
    if (selectedItem == -1 ||
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
    else if (event.getId() == "sell" && mAmountItems > 0
            && mAmountItems <= mMaxItems)
    {
        // Attempt sell
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(CMSG_NPC_SELL_REQUEST);
        outMsg.writeInt16(8);
        outMsg.writeInt16(mShopItems->at(selectedItem)->getInvIndex());
        outMsg.writeInt16(mAmountItems);

        mMaxItems -= mAmountItems;
        mShopItems->getShop()->at(selectedItem)->setQuantity(mMaxItems);
        mPlayerMoney +=
            mAmountItems * mShopItems->at(selectedItem)->getPrice();
        mAmountItems = 1;

        if (!mMaxItems)
        {
            // All were sold
            mShopItemList->setSelected(-1);
            mShopItems->getShop()->erase(
                    mShopItems->getShop()->begin() + selectedItem);

            gcn::Rectangle scroll;
            scroll.y = mShopItemList->getRowHeight() * (selectedItem + 1);
            scroll.height = mShopItemList->getRowHeight();
            mShopItemList->showPart(scroll);
        }
        else
        {
            mSlider->gcn::Slider::setScale(1, mMaxItems);
            // Update only when there are items left, the entry doesn't exist
            // otherwise and can't be updated
            updateButtonsAndLabels();
        }
    }
}

void SellDialog::valueChanged(const gcn::SelectionEvent &event)
{
    // Reset amount of items and update labels
    mAmountItems = 1;
    mSlider->setValue(0);

    updateButtonsAndLabels();
    mSlider->gcn::Slider::setScale(1, mMaxItems);
}

void SellDialog::widgetResized(const gcn::Event &event)
{
    Window::widgetResized(event);

    gcn::Rectangle area = getChildrenArea();
    int width = area.width;
    int height = area.height;

    mDecreaseButton->setPosition(8, height - 8 - mDecreaseButton->getHeight());
    mIncreaseButton->setPosition(
            mDecreaseButton->getX() + mDecreaseButton->getWidth() + 5,
            mDecreaseButton->getY());

    mQuitButton->setPosition(
            width - 8 - mQuitButton->getWidth(),
            height - 8 - mQuitButton->getHeight());
    mSellButton->setPosition(
            mQuitButton->getX() - 5 - mSellButton->getWidth(),
            mQuitButton->getY());

    mItemDescLabel->setDimension(gcn::Rectangle(8,
                mSellButton->getY() - 5 - mItemDescLabel->getHeight(),
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

void SellDialog::setMoney(int amount)
{
    mPlayerMoney = amount;
    mShopItemList->setPlayersMoney(amount);
}

void SellDialog::updateButtonsAndLabels()
{
    int selectedItem = mShopItemList->getSelected();
    int income = 0;

    if (selectedItem > -1)
    {
        const ItemInfo &info = mShopItems->at(selectedItem)->getInfo();
        mItemDescLabel->setCaption
            (strprintf(_("Description: %s"), info.getDescription().c_str()));
        mItemEffectLabel->setCaption
            (strprintf(_("Effect: %s"), info.getEffect().c_str()));

        mMaxItems = mShopItems->at(selectedItem)->getQuantity();
        if (mAmountItems > mMaxItems)
        {
            mAmountItems = mMaxItems;
        }

        income = mAmountItems * mShopItems->at(selectedItem)->getPrice();
    }
    else
    {
        mItemDescLabel->setCaption(strprintf(_("Description: %s"), ""));
        mItemEffectLabel->setCaption(strprintf(_("Effect: %s"), ""));
        mMaxItems = 0;
        mAmountItems = 0;
    }

    // Update Buttons and slider
    mSellButton->setEnabled(mAmountItems > 0);
    mDecreaseButton->setEnabled(mAmountItems > 1);
    mIncreaseButton->setEnabled(mAmountItems < mMaxItems);
    mSlider->setEnabled(mMaxItems > 1);

    // Update the quantity and money labels
    mQuantityLabel->setCaption(strprintf("%d / %d", mAmountItems, mMaxItems));
    mMoneyLabel->setCaption
        (strprintf(_("Price: %d GP / Total: %d GP"),
                   income, mPlayerMoney + income));
}
