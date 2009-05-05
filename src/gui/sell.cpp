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

#include "gui/sell.h"

#include "gui/shop.h"
#include "gui/shoplistbox.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/slider.h"

#include "npc.h"
#include "shopitem.h"
#include "units.h"

#include "net/net.h"
#include "net/npchandler.h"

#include "resources/iteminfo.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

SellDialog::SellDialog():
    Window(_("Sell")),
    mMaxItems(0), mAmountItems(0)
{
    setWindowName("Sell");
    setResizable(true);
    setCloseButton(true);
    setMinWidth(260);
    setMinHeight(230);
    setDefaultSize(260, 230, ImageRect::CENTER);

    // Create a ShopItems instance, that is aware of duplicate entries.
    mShopItems = new ShopItems(true);

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
    mSellButton = new Button(_("Sell"), "sell", this);
    mQuitButton = new Button(_("Quit"), "quit", this);
    mAddMaxButton = new Button(_("Max"), "max", this);
    mItemDescLabel = new Label(strprintf(_("Description: %s"), ""));
    mItemEffectLabel = new Label(strprintf(_("Effect: %s"), ""));

    mDecreaseButton->adjustSize();
    mDecreaseButton->setWidth(mIncreaseButton->getWidth());

    mIncreaseButton->setEnabled(false);
    mDecreaseButton->setEnabled(false);
    mSellButton->setEnabled(false);
    mSlider->setEnabled(false);

    mShopItemList->setPriceCheck(false);
    mShopItemList->addSelectionListener(this);
    mSlider->setActionEventId("slider");
    mSlider->addActionListener(this);

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
    place(6, 9, mSellButton);
    place(7, 9, mQuitButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    center();
    loadWindowState();
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

    mShopItems->addItem(item->getInvIndex(), item->getId(),
                        item->getQuantity(), price);

    mShopItemList->adjustSize();
}

void SellDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "quit")
    {
        close();
        return;
    }

    int selectedItem = mShopItemList->getSelected();

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
    else if (event.getId() == "max")
    {
        mAmountItems = mMaxItems;
        mSlider->setValue(mAmountItems);
        updateButtonsAndLabels();
    }
    else if (event.getId() == "sell" && mAmountItems > 0
            && mAmountItems <= mMaxItems)
    {
        // Attempt sell
        ShopItem *item = mShopItems->at(selectedItem);
        int sellCount, itemIndex;
        mPlayerMoney +=
            mAmountItems * mShopItems->at(selectedItem)->getPrice();
        mMaxItems -= mAmountItems;
        while (mAmountItems > 0) {
            // This order is important, item->getCurrentInvIndex() would return
            // the inventory index of the next Duplicate otherwise.
            itemIndex = item->getCurrentInvIndex();
            sellCount = item->sellCurrentDuplicate(mAmountItems);
            Net::getNpcHandler()->sellItem(current_npc, itemIndex, sellCount);
            mAmountItems -= sellCount;
        }

        mPlayerMoney +=
            mAmountItems * mShopItems->at(selectedItem)->getPrice();
        mAmountItems = 1;
        mSlider->setValue(0);

        if (!mMaxItems)
        {
            // All were sold
            mShopItemList->setSelected(-1);
            delete mShopItems->at(selectedItem);
            mShopItems->erase(selectedItem);

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
    mMoneyLabel->setCaption(strprintf(_("Price: %s / Total: %s"),
                    Units::formatCurrency(income).c_str(),
                    Units::formatCurrency(mPlayerMoney + income).c_str()));
}

void SellDialog::logic()
{
    Window::logic();

    if (!current_npc)
        setVisible(false);
}

void SellDialog::setVisible(bool visible)
{
    Window::setVisible(visible);

    if (visible)
        mShopItemList->requestFocus();
}

void SellDialog::close()
{
    setVisible(false);
    current_npc = 0;
}
