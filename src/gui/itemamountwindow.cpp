/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/itemamountwindow.h"

#include "inventory.h"
#include "item.h"
#include "keyboardconfig.h"

#include "gui/tradewindow.h"
#include "gui/itempopup.h"
#include "gui/viewport.h"

#include "gui/widgets/button.h"
#include "gui/widgets/inttextfield.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/slider.h"
#include "gui/widgets/icon.h"

#include "utils/gettext.h"

void ItemAmountWindow::finish(Item *item, int amount, Usage usage)
{
    switch (usage)
    {
        case TradeAdd:
            tradeWindow->tradeItem(item, amount);
            break;
        case ItemDrop:
            item->doEvent(Event::DoDrop, amount);
            break;
        case StoreAdd:
        {
            Event event(Event::DoMove);
            event.setItem("item", item);
            event.setInt("amount", amount);
            event.setInt("source", Inventory::INVENTORY);
            event.setInt("destination", Inventory::STORAGE);
            event.trigger(Event::ItemChannel);
        }
            break;
        case StoreRemove:
        {
            Event event(Event::DoMove);
            event.setItem("item", item);
            event.setInt("amount", amount);
            event.setInt("source", Inventory::STORAGE);
            event.setInt("destination", Inventory::INVENTORY);
            event.trigger(Event::ItemChannel);
        }
            break;
        default:
            break;
    }
}

ItemAmountWindow::ItemAmountWindow(Usage usage, Window *parent, Item *item,
                                   int maxRange):
    Window(std::string(), true, parent),
    mItem(item),
    mMax(maxRange),
    mUsage(usage)
{
    if (!mMax)
        mMax = mItem->getQuantity();

    // Save keyboard state
    mEnabledKeyboard = keyboard.isEnabled();
    keyboard.setEnabled(false);

    // Integer field
    mItemAmountTextField = new IntTextField(1);
    mItemAmountTextField->setRange(1, mMax);
    mItemAmountTextField->setWidth(35);
    mItemAmountTextField->addKeyListener(this);

    // Slider
    mItemAmountSlide = new Slider(1.0, mMax);
    mItemAmountSlide->setHeight(10);
    mItemAmountSlide->setActionEventId("slide");
    mItemAmountSlide->addActionListener(this);

    // Item icon
    mItemIcon = new Icon(item->getImage());

    // Buttons
    auto *minusButton = new Button(_("-"), "dec", this);
    auto *plusButton = new Button(_("+"), "inc", this);
    auto *okButton = new Button(_("OK"), "ok", this);
    auto *cancelButton = new Button(_("Cancel"), "cancel", this);
    auto *addAllButton = new Button(_("All"), "all", this);

    minusButton->adjustSize();
    minusButton->setWidth(plusButton->getWidth());

    // Set positions
    ContainerPlacer place;
    place = getPlacer(0, 0);
    place(1, 0, minusButton);
    place(2, 0, mItemAmountTextField);
    place(3, 0, plusButton);
    place(4, 0, addAllButton);

    place(0, 0, mItemIcon, 1, 3);
    place(1, 1, mItemAmountSlide, 5);

    place(4, 2, cancelButton);
    place(5, 2, okButton);

    reflowLayout();

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
    }

    setLocationRelativeTo(getParentWindow());
    setVisible(true);

    mItemPopup = new ItemPopup;
    mItemIcon->addMouseListener(this);
}

ItemAmountWindow::~ItemAmountWindow()
{
    delete mItemPopup;
}

// Show ItemTooltip
void ItemAmountWindow::mouseMoved(gcn::MouseEvent &event)
{
    if (event.getSource() == mItemIcon)
    {
        mItemPopup->setItem(mItem->getInfo());
        mItemPopup->position(viewport->getMouseX(), viewport->getMouseY());
    }
}

// Hide ItemTooltip
void ItemAmountWindow::mouseExited(gcn::MouseEvent &event)
{
    mItemPopup->setVisible(false);
}

void ItemAmountWindow::resetAmount()
{
    mItemAmountTextField->setValue(1);
}

void ItemAmountWindow::action(const gcn::ActionEvent &event)
{
    int amount = mItemAmountTextField->getValue();

    if (event.getId() == "cancel")
    {
        close();
    }
    else if (event.getId() == "inc" && amount < mMax)
    {
        amount++;
    }
    else if (event.getId() == "dec" && amount > 1)
    {
        amount--;
    }
    else if (event.getId() == "all")
    {
        amount = mMax;
    }
    else if (event.getId() == "slide")
    {
        amount = static_cast<int>(mItemAmountSlide->getValue());
    }
    else if (event.getId() == "ok")
    {
        finish(mItem, amount, mUsage);
        close();
        return;
    }
    mItemAmountTextField->setValue(amount);
    mItemAmountSlide->setValue(amount);
}

void ItemAmountWindow::close()
{
    keyboard.setEnabled(mEnabledKeyboard);
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
