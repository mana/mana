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

#include "button.h"
#include "item_amount.h"
#include "label.h"
#include "slider.h"
#include "storagewindow.h"
#include "trade.h"

#include "widgets/layout.h"

#include "../item.h"
#include "../localplayer.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"

ItemAmountWindow::ItemAmountWindow(int usage, Window *parent, Item *item):
    Window("", true, parent),
    mItem(item),
    mMax(item->getQuantity()),
    mUsage(usage)
{
    setCloseButton(true);

    // Integer field

    mItemAmountLabel = new Label(strprintf("%d / %d", 1, mMax));
    mItemAmountLabel->setAlignment(gcn::Graphics::CENTER);

    // Slider
    mItemAmountSlide = new Slider(1.0, mMax);
    mItemAmountSlide->setHeight(10);
    mItemAmountSlide->setActionEventId("Slide");
    mItemAmountSlide->addActionListener(this);

    // Buttons
    Button *minusButton = new Button("-", "Minus", this);
    Button *plusButton = new Button("+", "Plus", this);
    Button *okButton = new Button(_("Ok"), "Ok", this);
    Button *cancelButton = new Button(_("Cancel"), "Cancel", this);
    Button *addAllButton = new Button(_("All"), "All", this);

    minusButton->adjustSize();
    minusButton->setWidth(plusButton->getWidth());

    // Set positions
    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, minusButton);
    place(1, 0, mItemAmountSlide, 3);
    place(4, 0, plusButton);
    place(5, 0, mItemAmountLabel, 2);
    place(7, 0, addAllButton);
    place = getPlacer(0, 1);
    place(4, 0, cancelButton);
    place(5, 0, okButton);
    reflowLayout(225, 0);

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
    mItemAmountLabel->setCaption(strprintf("%d / %d", 1, mMax));
}

void ItemAmountWindow::action(const gcn::ActionEvent &event)
{
    int amount = mItemAmountSlide->getValue();

    if (event.getId() == "Cancel")
    {
        close();
    }
    else if (event.getId() == "Plus" && amount < mMax)
    {
        amount++;
    }
    else if (event.getId() == "Minus" && amount > 1)
    {
        amount--;
    }
    else if (event.getId() == "Slide")
    {
        amount = static_cast<int>(mItemAmountSlide->getValue());
    }
    else if (event.getId() == "Ok" || event.getId() == "All")
    {
        if (event.getId() == "All") 
            amount = mMax;

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
        return;
    }

    mItemAmountLabel->setCaption(strprintf("%d / %d", amount, mMax));
    mItemAmountSlide->setValue(amount);
}

void ItemAmountWindow::close()
{
    scheduleDelete();
}
