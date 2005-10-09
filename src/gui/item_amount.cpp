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
 *  $Id $
 */

#include "item_amount.h"

#include "button.h"
#include "inttextbox.h"
#include "slider.h"
#include "trade.h"

#include "../inventory.h"
#include "../item.h"

extern Inventory *inventory;

ItemAmountWindow::ItemAmountWindow(int usage, Window *parent, Item *item):
    Window("Select amount of items to drop.", true, parent),
    mItem(item)
{
    // New labels
    mItemAmountTextBox = new IntTextBox(1);

    // New buttons
    mItemAmountMinusButton = new Button("-");
    mItemAmountPlusButton = new Button("+");
    mItemAmountSlide = new Slider(1.0);
    mItemAmountOkButton = new Button("Okay");
    mItemAmountCancelButton = new Button("Cancel");

    mItemAmountTextBox->setRange(1, mItem->getQuantity());
    mItemAmountSlide->setDimension(gcn::Rectangle(5, 120, 180, 10));

    // Set button events Id
    mItemAmountMinusButton->setEventId("Minus");
    mItemAmountPlusButton->setEventId("Plus");
    mItemAmountSlide->setEventId("Slide");
    mItemAmountOkButton->setEventId("Drop");
    mItemAmountCancelButton->setEventId("Cancel");

    // Set position
    mItemAmountTextBox->setPosition(35, 10);
    mItemAmountTextBox->setSize(24, 16);
    mItemAmountPlusButton->setPosition(60, 5);
    mItemAmountMinusButton->setPosition(10, 5);
    mItemAmountSlide->setPosition(10, 35);
    mItemAmountOkButton->setPosition(10, 50);
    mItemAmountCancelButton->setPosition(60, 50);

    // Assemble
    add(mItemAmountTextBox);
    add(mItemAmountPlusButton);
    add(mItemAmountMinusButton);
    add(mItemAmountSlide);
    add(mItemAmountOkButton);
    add(mItemAmountCancelButton);

    mItemAmountPlusButton->addActionListener(this);
    mItemAmountMinusButton->addActionListener(this);
    mItemAmountSlide->addActionListener(this);
    mItemAmountOkButton->addActionListener(this);
    mItemAmountCancelButton->addActionListener(this);

    resetAmount();

    switch (usage) {
        case AMOUNT_TRADE_ADD:
            setCaption("Select amount of items to trade.");
            mItemAmountOkButton->setEventId("AddTrade");
            break;
        case AMOUNT_ITEM_DROP:
            setCaption("Select amount of items to drop.");
            mItemAmountOkButton->setEventId("Drop");
            break;
        default:
            break;
    }

    setContentSize(200, 80);
    setLocationRelativeTo(getParentWindow());
}

ItemAmountWindow::~ItemAmountWindow()
{
    delete mItemAmountTextBox;
    delete mItemAmountPlusButton;
    delete mItemAmountMinusButton;
    delete mItemAmountSlide;
    delete mItemAmountOkButton;
    delete mItemAmountCancelButton;
}

void ItemAmountWindow::resetAmount()
{
    mItemAmountTextBox->setInt(1);
}

void ItemAmountWindow::action(const std::string& eventId)
{

    if (eventId == "Cancel")
    {
        scheduleDelete();
    }
    else if (eventId == "Drop")
    {
        inventory->dropItem(mItem, mItemAmountTextBox->getInt());
        scheduleDelete();
    }
    else if (eventId == "AddTrade")
    {
        tradeWindow->tradeItem(mItem, mItemAmountTextBox->getInt());
        scheduleDelete();
    }
    else if (eventId == "Plus")
    {
        mItemAmountTextBox->setInt(mItemAmountTextBox->getInt() + 1);
    }
    else if (eventId == "Minus")
    {
        mItemAmountTextBox->setInt(mItemAmountTextBox->getInt() - 1);
    }
    else if (eventId == "Slide")
    {
        mItemAmountTextBox->setInt((int)(mItemAmountSlide->getValue()*mItem->getQuantity()));
    }
}

