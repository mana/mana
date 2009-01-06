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

#include "item_amount.h"

#include "button.h"
#include "inttextbox.h"
#include "slider.h"
#include "trade.h"

#include "widgets/layout.h"

#include "../item.h"
#include "../localplayer.h"

#include "../utils/gettext.h"

ItemAmountWindow::ItemAmountWindow(int usage, Window *parent, Item *item):
    Window("", true, parent),
    mItem(item)
{
    const int maxRange = mItem->getQuantity();

    // Integer field
    mItemAmountTextBox = new IntTextBox(1);
    mItemAmountTextBox->setRange(1, maxRange);
    mItemAmountTextBox->setWidth(30);
    mItemAmountTextBox->setActionEventId("Dummy");
    mItemAmountTextBox->addActionListener(this);

    // Slider
    mItemAmountSlide = new Slider(1.0, maxRange);
    mItemAmountSlide->setHeight(10);
    mItemAmountSlide->setActionEventId("Slide");
    mItemAmountSlide->addActionListener(this);

    // Buttons
    Button *minusButton = new Button("-", "Minus", this);
    minusButton->setSize(20, 20);
    Button *plusButton = new Button("+", "Plus", this);
    plusButton->setSize(20, 20);
    Button *okButton = new Button(_("Ok"), "Drop", this);
    Button *cancelButton = new Button(_("Cancel"), "Cancel", this);

    // Set positions
    place(0, 0, minusButton);
    place(1, 0, mItemAmountTextBox).setPadding(2);
    place(2, 0, plusButton);
    place(0, 1, mItemAmountSlide, 6);
    place(4, 2, okButton);
    place(5, 2, cancelButton);
    reflowLayout(250, 0);

    resetAmount();

    switch (usage) {
        case AMOUNT_TRADE_ADD:
            setCaption(_("Select amount of items to trade."));
            okButton->setActionEventId("AddTrade");
            break;
        case AMOUNT_ITEM_DROP:
            setCaption(_("Select amount of items to drop."));
            okButton->setActionEventId("Drop");
            break;
        default:
            break;
    }

    setLocationRelativeTo(getParentWindow());
    setVisible(true);
}

void ItemAmountWindow::resetAmount()
{
    mItemAmountTextBox->setInt(1);
}

void ItemAmountWindow::action(const gcn::ActionEvent &event)
{
    int amount = mItemAmountTextBox->getInt();

    if (event.getId() == "Cancel")
    {
        scheduleDelete();
    }
    else if (event.getId() == "Plus")
    {
        amount++;
    }
    else if (event.getId() == "Minus")
    {
        amount--;
    }
    else if (event.getId() == "Slide")
    {
        amount = static_cast<int>(mItemAmountSlide->getValue());
    }
    else if (event.getId() == "Drop")
    {
        player_node->dropItem(mItem, mItemAmountTextBox->getInt());
        scheduleDelete();
    }
    else if (event.getId() == "AddTrade")
    {
        tradeWindow->tradeItem(mItem, mItemAmountTextBox->getInt());
        scheduleDelete();
    }
    mItemAmountTextBox->setInt(amount);
    mItemAmountSlide->setValue(amount);
}
