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

#include "buy.h"

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "scrollarea.h"
#include "shop.h"
#include "shoplistbox.h"
#include "slider.h"

#include "widgets/layout.h"

#include "../npc.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"

BuyDialog::BuyDialog(Network *network):
    Window(_("Buy")), mNetwork(network),
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
    mMoneyLabel = new gcn::Label(strprintf(_("Price: %d GP / Total: %d GP"), 0, 0));
    mIncreaseButton = new Button("+", "+", this);
    mDecreaseButton = new Button("-", "-", this);
    mBuyButton = new Button(_("Buy"), "buy", this);
    mQuitButton = new Button(_("Quit"), "quit", this);
    mItemDescLabel = new gcn::Label(strprintf(_("Description: %s"), ""));
    mItemEffectLabel = new gcn::Label(strprintf(_("Effect: %s"), ""));

    mIncreaseButton->setSize(20, 20);
    mDecreaseButton->setSize(20, 20);

    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mIncreaseButton->setEnabled(false);
    mDecreaseButton->setEnabled(false);
    mBuyButton->setEnabled(false);
    mSlider->setEnabled(false);

    mSlider->setActionEventId("slider");
    mSlider->addActionListener(this);
    mShopItemList->addSelectionListener(this);

    place(0, 0, mScrollArea, 5).setPadding(3);
    place(0, 1, mQuantityLabel, 2);
    place(2, 1, mSlider, 3);
    place(0, 2, mMoneyLabel, 5);
    place(0, 3, mItemEffectLabel, 5);
    place(0, 4, mItemDescLabel, 5);
    place(0, 5, mDecreaseButton);
    place(1, 5, mIncreaseButton);
    place(3, 5, mBuyButton);
    place(4, 5, mQuitButton);
    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

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
    // there a better way to ensure this fails in an _obvious_ way in C++?
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

void BuyDialog::updateButtonsAndLabels()
{
    const int selectedItem = mShopItemList->getSelected();
    int price = 0;

    if (selectedItem > -1)
    {
        const ItemInfo &info = mShopItems->at(selectedItem)->getInfo();

        mItemDescLabel->setCaption
            (strprintf(_("Description: %s"), info.getDescription().c_str()));
        mItemEffectLabel->setCaption
            (strprintf(_("Effect: %s"), info.getEffect().c_str()));

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
        mItemDescLabel->setCaption(strprintf(_("Description: %s"), ""));
        mItemEffectLabel->setCaption(strprintf(_("Effect: %s"), ""));
        mMaxItems = 0;
        mAmountItems = 0;
    }

    // Enable or disable buttons and slider
    mIncreaseButton->setEnabled(mAmountItems < mMaxItems);
    mDecreaseButton->setEnabled(mAmountItems > 1);
    mBuyButton->setEnabled(mAmountItems > 0);
    mSlider->setEnabled(mMaxItems > 1);

    // Update quantity and money labels
    mQuantityLabel->setCaption(strprintf("%d / %d", mAmountItems, mMaxItems));
    mMoneyLabel->setCaption
        (strprintf(_("Price: %d GP / Total: %d GP"), price, mMoney - price));
}
