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

#include "gui/itemamount.h"

#include "gui/storagewindow.h"
#include "gui/trade.h"

#include "gui/widgets/button.h"
#include "gui/widgets/inttextfield.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/slider.h"

#include "item.h"
#include "localplayer.h"

#include "utils/gettext.h"

void ItemAmountWindow::finish(Item *item, int amount, Usage usage)
{
    switch (usage)
    {
        case TradeAdd:
            tradeWindow->tradeItem(item, amount);
            break;
        case ItemDrop:
            player_node->dropItem(item, amount);
            break;
        case ItemSplit:
            player_node->splitItem(item, amount);
            break;
        case StoreAdd:
            storageWindow->addStore(item, amount);
            break;
        case StoreRemove:
            storageWindow->removeStore(item, amount);
            break;
        default:
            break;
    }
}

ItemAmountWindow::ItemAmountWindow(Usage usage, Window *parent, Item *item,
                                   int maxRange):
    Window("", true, parent),
    mItem(item),
    mMax(maxRange),
    mUsage(usage)
{
    if (!mMax)
        mMax = mItem->getQuantity();

    setCloseButton(true);

    // Integer field
    mItemAmountTextField = new IntTextField(1);
    mItemAmountTextField->setRange(1, mMax);
    mItemAmountTextField->setWidth(35);
    mItemAmountTextField->addKeyListener(this);

    // Slider
    mItemAmountSlide = new Slider(1.0, mMax);
    mItemAmountSlide->setHeight(10);
    mItemAmountSlide->setActionEventId("Slide");
    mItemAmountSlide->addActionListener(this);

    // Buttons
    Button *minusButton = new Button("-", "Minus", this);
    Button *plusButton = new Button("+", "Plus", this);
    Button *okButton = new Button(_("Ok"), "Ok", this);
    Button *addAllButton = new Button(_("All"), "All", this);

    minusButton->adjustSize();
    minusButton->setWidth(plusButton->getWidth());

    // Set positions
    ContainerPlacer place;
    place = getPlacer(0, 0);
    place(0, 0, minusButton);
    place(1, 0, mItemAmountTextField);
    place(2, 0, plusButton);
    place(3, 0, addAllButton);
    place = getPlacer(0, 1);
    place(0, 0, mItemAmountSlide, 6);
    place = getPlacer(0, 2);
    place(5, 0, okButton);
    reflowLayout(225, 0);

    resetAmount();

    switch (usage)
    {
        case TradeAdd:
            setCaption(_("Select amount of items to trade."));
            break;
        case ItemDrop:
            setCaption(_("Select amount of items to drop."));
            break;
        case StoreAdd:
            setCaption(_("Select amount of items to store."));
            break;
        case StoreRemove:
            setCaption(_("Select amount of items to retrieve."));
            break;
        case ItemSplit:
            setCaption(_("Select amount of items to split."));
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

    if (event.getId() == "Plus" && amount < mMax)
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

        finish(mItem, amount, mUsage);

        scheduleDelete();
        return;
    }
    mItemAmountTextField->setValue(amount);
    mItemAmountSlide->setValue(amount);
}

void ItemAmountWindow::close()
{
    scheduleDelete();
}

void ItemAmountWindow::keyReleased(gcn::KeyEvent &keyEvent)
{
    mItemAmountSlide->setValue(mItemAmountTextField->getValue());
}

void ItemAmountWindow::showWindow(Usage usage, Window *parent, Item *item,
                                  int maxRange)
{
    if (!maxRange)
        maxRange = item->getQuantity();

    if (maxRange <= 1)
    {
        finish(item, maxRange, usage);
    }
    else
    {
        new ItemAmountWindow(usage, parent, item, maxRange);
    }
}
