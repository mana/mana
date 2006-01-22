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

#include "../item.h"
#include "../localplayer.h"

ItemAmountWindow::ItemAmountWindow(int usage, Window *parent, Item *item):
    Window("Select amount of items to drop.", true, parent),
    mItem(item)
{
    // New labels
    mItemAmountTextBox = new IntTextBox(1);

    // New buttons
    Button *minusButton = new Button("-");
    Button *plusButton = new Button("+");
    mItemAmountSlide = new Slider(1.0);
    Button *okButton = new Button("Okay");
    Button *cancelButton = new Button("Cancel");

    mItemAmountTextBox->setRange(1, mItem->getQuantity());
    mItemAmountSlide->setDimension(gcn::Rectangle(5, 120, 180, 10));

    // Set button events Id
    minusButton->setEventId("Minus");
    plusButton->setEventId("Plus");
    mItemAmountSlide->setEventId("Slide");
    okButton->setEventId("Drop");
    cancelButton->setEventId("Cancel");

    // Set position
    mItemAmountTextBox->setPosition(35, 10);
    mItemAmountTextBox->setSize(24, 16);
    plusButton->setPosition(60, 5);
    minusButton->setPosition(10, 5);
    mItemAmountSlide->setPosition(10, 35);
    okButton->setPosition(10, 50);
    cancelButton->setPosition(60, 50);

    // Assemble
    add(mItemAmountTextBox);
    add(plusButton);
    add(minusButton);
    add(mItemAmountSlide);
    add(okButton);
    add(cancelButton);

    plusButton->addActionListener(this);
    minusButton->addActionListener(this);
    mItemAmountSlide->addActionListener(this);
    okButton->addActionListener(this);
    cancelButton->addActionListener(this);

    resetAmount();

    switch (usage) {
        case AMOUNT_TRADE_ADD:
            setCaption("Select amount of items to trade.");
            okButton->setEventId("AddTrade");
            break;
        case AMOUNT_ITEM_DROP:
            setCaption("Select amount of items to drop.");
            okButton->setEventId("Drop");
            break;
        default:
            break;
    }

    setContentSize(200, 80);
    setLocationRelativeTo(getParentWindow());
}

void ItemAmountWindow::resetAmount()
{
    mItemAmountTextBox->setInt(1);
}

void ItemAmountWindow::action(const std::string& eventId)
{
    int amount = mItemAmountTextBox->getInt();

    if (eventId == "Cancel")
    {
        scheduleDelete();
    }
    else if (eventId == "Drop")
    {
        player_node->dropItem(mItem, mItemAmountTextBox->getInt());
        scheduleDelete();
    }
    else if (eventId == "AddTrade")
    {
        tradeWindow->tradeItem(mItem, mItemAmountTextBox->getInt());
        scheduleDelete();
    }
    else if (eventId == "Plus")
    {
        amount++;
    }
    else if (eventId == "Minus")
    {
        amount--;
    }
    else if (eventId == "Slide")
    {
        amount = (int)(mItemAmountSlide->getValue()*(mItem->getQuantity()+1));
    }
    mItemAmountTextBox->setInt(amount);
    amount = mItemAmountTextBox->getInt(); // The textbox cares about bounds
    mItemAmountSlide->setValue((amount-1)*1.0f/(mItem->getQuantity()-1));
}

