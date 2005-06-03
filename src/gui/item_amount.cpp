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
#include "../engine.h"
#include "../main.h"

ItemAmountWindow::ItemAmountWindow(int usage, Window *parent):
    Window("Select amount of items to drop.", true, parent)
{
    // New labels
    itemAmountTextBox = new IntTextBox(1);

    // New buttons
    itemAmountMinusButton = new Button("-");
    itemAmountPlusButton = new Button("+");
    itemAmountOkButton = new Button("Okay");
    itemAmountCancelButton = new Button("Cancel");

    itemAmountTextBox->setRange(1, inventoryWindow->items->getQuantity());

    // Set button events Id
    itemAmountMinusButton->setEventId("Minus");
    itemAmountPlusButton->setEventId("Plus");
    itemAmountOkButton->setEventId("Drop");
    itemAmountCancelButton->setEventId("Cancel");

    // Set position
    itemAmountTextBox->setPosition(35, 10);
    itemAmountTextBox->setSize(24, 16);
    itemAmountPlusButton->setPosition(60, 5);
    itemAmountMinusButton->setPosition(10, 5);
    itemAmountOkButton->setPosition(10, 40);
    itemAmountCancelButton->setPosition(60, 40);

    // Assemble
    add(itemAmountTextBox);
    add(itemAmountPlusButton);
    add(itemAmountMinusButton);
    add(itemAmountOkButton);
    add(itemAmountCancelButton);

    itemAmountPlusButton->addActionListener(this);
    itemAmountMinusButton->addActionListener(this);
    itemAmountOkButton->addActionListener(this);
    itemAmountCancelButton->addActionListener(this);

    resetAmount();

    switch (usage) {
        case AMOUNT_TRADE_ADD:
            setCaption("Select amount of items to trade.");
            itemAmountOkButton->setEventId("AddTrade");
            break;
        case AMOUNT_ITEM_DROP:
            setCaption("Select amount of items to drop.");
            itemAmountOkButton->setEventId("Drop");
            break;
        default:
            break;
    }

    setContentSize(200, 80);
    setLocationRelativeTo(getParentWindow());
}

ItemAmountWindow::~ItemAmountWindow()
{
    delete itemAmountTextBox;
    delete itemAmountPlusButton;
    delete itemAmountMinusButton;
    delete itemAmountOkButton;
    delete itemAmountCancelButton;
}

void ItemAmountWindow::resetAmount()
{
    itemAmountTextBox->setInt(1);
}

void ItemAmountWindow::action(const std::string& eventId)
{

    if (eventId == "Cancel")
    {
        scheduleDelete();
    }
    else if (eventId == "Drop")
    {
        inventoryWindow->dropItem(inventoryWindow->items->getIndex(), itemAmountTextBox->getInt());
        scheduleDelete();
    }
    else if (eventId == "AddTrade")
    {
        tradeWindow->tradeItem(inventoryWindow->items->getIndex(), itemAmountTextBox->getInt());
        scheduleDelete();
    }
    else if (eventId == "Plus")
    {
        itemAmountTextBox->setInt(itemAmountTextBox->getInt() + 1);
    }
    else if (eventId == "Minus")
    {
        itemAmountTextBox->setInt(itemAmountTextBox->getInt() - 1);
    }
}

