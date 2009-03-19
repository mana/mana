/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

#include "button.h"
#include "inttextfield.h"
#include "item_amount.h"
#include "slider.h"
#include "storagewindow.h"
#include "trade.h"

#include "widgets/layout.h"

#include "../item.h"
#include "../localplayer.h"

#include "../utils/gettext.h"

ItemAmountWindow::ItemAmountWindow(int usage, Window *parent, Item *item):
    Window("", true, parent),
    mItem(item), mUsage(usage), mMax(mItem->getQuantity())
{
    // Integer field
    mItemAmountTextField = new IntTextField(1);
    mItemAmountTextField->setRange(1, mMax);
    mItemAmountTextField->setWidth(30);
    mItemAmountTextField->setActionEventId("Dummy");
    mItemAmountTextField->addActionListener(this);

    // Slider
    mItemAmountSlide = new Slider(1.0, mMax);
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
    Button *addAllButton = new Button(_("All"), "All", this);

    // Set positions
    place(0, 0, minusButton);
    place(1, 0, mItemAmountTextField).setPadding(2);
    place(2, 0, plusButton);
    place(4, 0, addAllButton, 2);
    place(0, 1, mItemAmountSlide, 6);
    place(4, 2, okButton);
    place(5, 2, cancelButton);
    reflowLayout(250, 0);

    resetAmount();

    switch (usage)
    {
        case AMOUNT_TRADE_ADD:
            setCaption(_("Select amount of items to trade."));
            break;
        case AMOUNT_ITEM_DROP:
            setCaption(_("Select amount of items to drop."));
            break;
        case AMOUNT_STORE_ADD:
            setCaption(_("Select amount of items to store."));
            break;
        case AMOUNT_STORE_REMOVE:
            setCaption(_("Select amount of items to retrieve."));
            break;
        default:
            break;
    }

    setLocationRelativeTo(getParentWindow());
    setVisible(true);
}

void ItemAmountWindow::resetAmount()
{
    mItemAmountTextField->setValue(1);
}

void ItemAmountWindow::action(const gcn::ActionEvent &event)
{
    int amount = mItemAmountTextField->getValue();

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
    else if (event.getId() == "Ok" || event.getId() == "All")
    {
        if (event.getId() == "All") {
            amount = mMax;
        }

        switch (mUsage)
        {
            case AMOUNT_TRADE_ADD:
                tradeWindow->tradeItem(mItem, amount);
                break;
            case AMOUNT_ITEM_DROP:
                player_node->dropItem(mItem, amount);
                break;
            case AMOUNT_STORE_ADD:
                storageWindow->addStore(mItem, amount);
                break;
            case AMOUNT_STORE_REMOVE:
                storageWindow->removeStore(mItem, amount);
                break;
            default:
                return;
                break;
        }

        scheduleDelete();
    }

    mItemAmountTextField->setValue(amount);
    mItemAmountSlide->setValue(amount);
}
