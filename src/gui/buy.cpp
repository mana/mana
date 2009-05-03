/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gui/buy.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/slider.h"

#include "gui/shop.h"
#include "gui/shoplistbox.h"

#include "npc.h"
#include "shopitem.h"
#include "units.h"

#include "net/net.h"
#include "net/npchandler.h"

#include "resources/iteminfo.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

BuyDialog::BuyDialog():
    Window(_("Buy")),
    mMoney(0), mAmountItems(0), mMaxItems(0)
{
    setWindowName("Buy");
    setResizable(true);
    setCloseButton(true);
    setMinWidth(260);
    setMinHeight(230);
    setDefaultSize(260, 230, ImageRect::CENTER);

    mShopItems = new ShopItems;

    mShopItemList = new ShopListBox(mShopItems, mShopItems);
    mScrollArea = new ScrollArea(mShopItemList);
    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mSlider = new Slider(1.0);
    mQuantityLabel = new Label(strprintf("%d / %d", mAmountItems, mMaxItems));
    mQuantityLabel->setAlignment(gcn::Graphics::CENTER);
    mMoneyLabel = new Label(strprintf(_("Price: %s / Total: %s"),
                                            "", ""));

    mIncreaseButton = new Button("+", "+", this);
    mDecreaseButton = new Button("-", "-", this);
    mBuyButton = new Button(_("Buy"), "buy", this);
    mQuitButton = new Button(_("Quit"), "quit", this);
    mAddMaxButton = new Button(_("Max"), "max", this);
    mItemDescLabel = new Label(strprintf(_("Description: %s"), ""));
    mItemEffectLabel = new Label(strprintf(_("Effect: %s"), ""));

    mDecreaseButton->adjustSize();
    mDecreaseButton->setWidth(mIncreaseButton->getWidth());

    mIncreaseButton->setEnabled(false);
    mDecreaseButton->setEnabled(false);
    mBuyButton->setEnabled(false);
    mSlider->setEnabled(false);

    mSlider->setActionEventId("slider");
    mSlider->addActionListener(this);
    mShopItemList->addSelectionListener(this);

    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mScrollArea, 8, 5).setPadding(3);
    place(0, 5, mDecreaseButton);
    place(1, 5, mSlider, 3);
    place(4, 5, mIncreaseButton);
    place(5, 5, mQuantityLabel, 2);
    place(7, 5, mAddMaxButton);
    place(0, 6, mMoneyLabel, 8);
    place(0, 7, mItemEffectLabel, 8);
    place(0, 8, mItemDescLabel, 8);
    place(6, 9, mBuyButton);
    place(7, 9, mQuitButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    center();
    loadWindowState();
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

void BuyDialog::addItem(int id, int amount, int price)
{
    mShopItems->addItem(id, amount, price);
    mShopItemList->adjustSize();
}

void BuyDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "quit")
    {
        close();
        return;
    }

    int selectedItem = mShopItemList->getSelected();

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
    else if (event.getId() == "max")
    {
        mAmountItems = mMaxItems;
        mSlider->setValue(mAmountItems);
        updateButtonsAndLabels();
    }
    // TODO: Actually we'd have a bug elsewhere if this check for the number
    // of items to be bought ever fails, Bertram removed the assertions, is
    // there a better way to ensure this fails in an _obvious_ way in C++?
    else if (event.getId() == "buy" && mAmountItems > 0 &&
                mAmountItems <= mMaxItems)
    {
        Net::getNpcHandler()->buyItem(current_npc,
                                      mShopItems->at(selectedItem)->getId(),
                                      mAmountItems);

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
        (strprintf(_("Price: %s / Total: %s"),
                    Units::formatCurrency(price).c_str(),
                    Units::formatCurrency(mMoney - price).c_str()));
}

void BuyDialog::logic()
{
    Window::logic();

    if (!current_npc) setVisible(false);
}

void BuyDialog::setVisible(bool visible)
{
    Window::setVisible(visible);

    if (visible)
        mShopItemList->requestFocus();
}

void BuyDialog::close()
{
    setVisible(false);
    current_npc = 0;
}
