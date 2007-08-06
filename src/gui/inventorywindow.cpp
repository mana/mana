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
 *  $Id$
 */

#include "inventorywindow.h"

#include <string>

#include <guichan/mouseinput.hpp>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "gui.h"
#include "item_amount.h"
#include "itemcontainer.h"
#include "scrollarea.h"
#include "viewport.h"

#include "../item.h"
#include "../localplayer.h"

#include "../resources/iteminfo.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"

InventoryWindow::InventoryWindow():
    Window(_("Inventory"))
{
    setResizable(true);
    setMinWidth(240);
    setMinHeight(172);
    // If you adjust these defaults, don't forget to adjust the trade window's.
    setDefaultSize(115, 25, 322, 172);

    mUseButton = new Button(_("Use"), "use", this);
    mDropButton = new Button(_("Drop"), "drop", this);

    mItems = new ItemContainer(player_node->mInventory.get());
    mItems->addSelectionListener(this);

    mInvenScroll = new ScrollArea(mItems);
    mInvenScroll->setPosition(8, 8);
    mInvenScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mItemNameLabel = new gcn::Label(strprintf(_("Name: %s"), ""));
    mItemDescriptionLabel = new gcn::Label(strprintf(_("Description: %s"), ""));
    mItemEffectLabel = new gcn::Label(strprintf(_("Effect: %s"), ""));
    mWeightLabel = new gcn::Label(strprintf(_("Total Weight: %d - Maximum Weight: %d"), 0, 0));
    mWeightLabel->setPosition(8, 8);
    mInvenScroll->setPosition(8,
            mWeightLabel->getY() + mWeightLabel->getHeight() + 5);

    add(mUseButton);
    add(mDropButton);
    add(mInvenScroll);
    add(mItemNameLabel);
    add(mItemDescriptionLabel);
    add(mItemEffectLabel);
    add(mWeightLabel);

    mUseButton->setSize(48, mUseButton->getHeight());

    loadWindowState();
}

void InventoryWindow::logic()
{
    Window::logic();

    // It would be nicer if this update could be event based, needs some
    // redesign of InventoryWindow and ItemContainer probably.
    updateButtons();

    // Update weight information
    mWeightLabel->setCaption(strprintf(_("Total Weight: %d - Maximum Weight: %d"),
        player_node->getTotalWeight(), player_node->getMaxWeight()));
    mWeightLabel->adjustSize();
}

void InventoryWindow::action(const gcn::ActionEvent &event)
{
    Item *item = mItems->getItem();

    if (!item) {
        return;
    }

    if (event.getId() == "use") {
        if (item->isEquipment()) {
            player_node->equipItem(item);
        }
        else {
            player_node->useItem(item);
        }
    }
    else if (event.getId() == "drop")
    {
        // Choose amount of items to drop
        new ItemAmountWindow(AMOUNT_ITEM_DROP, this, item);
    }
}

void InventoryWindow::selectionChanged(const SelectionEvent &event)
{
    Item *item = mItems->getItem();
    ItemInfo const *info = item ? &item->getInfo() : NULL;

    mItemNameLabel->setCaption
        (strprintf(_("Name: %s"), info ? info->getName().c_str() : ""));
    mItemEffectLabel->setCaption
        (strprintf(_("Effect: %s"), info ? info->getEffect().c_str() : ""));
    mItemDescriptionLabel->setCaption
        (strprintf(_("Description: %s"), info ? info->getDescription().c_str() : ""));

    mItemNameLabel->adjustSize();
    mItemEffectLabel->adjustSize();
    mItemDescriptionLabel->adjustSize();
}

void InventoryWindow::mouseClicked(gcn::MouseEvent &event)
{
    Window::mouseClicked(event);

    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Item *item = mItems->getItem();

        if (!item) return;

        /* Convert relative to the window coordinates to absolute screen
         * coordinates.
         */
        int mx = event.getX() + getX();
        int my = event.getY() + getY();
        viewport->showPopup(mx, my, item);
    }
}

void InventoryWindow::mouseDragged(gcn::MouseEvent &event)
{
    int tmpWidth = getWidth(), tmpHeight = getHeight();
    Window::mouseDragged(event);
    if (getWidth() != tmpWidth || getHeight() != tmpHeight)
    {
        updateWidgets();
    }
}

void InventoryWindow::updateWidgets()
{
    gcn::Rectangle area = getChildrenArea();
    int width = area.width;
    int height = area.height;
    int columns = width / 24;

    if (columns < 1)
    {
        columns = 1;
    }

    // Resize widgets
    mUseButton->setPosition(8, height - 24);
    mDropButton->setPosition(48 + 16, height - 24);
    mInvenScroll->setSize(width - 16, height - 110);

    mItemNameLabel->setPosition(8,
            mInvenScroll->getY() + mInvenScroll->getHeight() + 4);
    mItemEffectLabel->setPosition(8,
            mItemNameLabel->getY() + mItemNameLabel->getHeight() + 4);
    mItemDescriptionLabel->setPosition(8,
            mItemEffectLabel->getY() + mItemEffectLabel->getHeight() + 4);
}

void InventoryWindow::updateButtons()
{
    Item *item;

    if ((item = mItems->getItem()) && item->isEquipment())
    {
        mUseButton->setCaption(_("Equip"));
    }
    else {
        mUseButton ->setCaption(_("Use"));
    }

    mUseButton->setEnabled(!!item);
    mDropButton->setEnabled(!!item);
}

Item* InventoryWindow::getItem()
{
    return mItems->getItem();
}

void InventoryWindow::loadWindowState()
{
    Window::loadWindowState();
    updateWidgets();
}

void InventoryWindow::resetToDefaultSize()
{
    Window::resetToDefaultSize();
    updateWidgets();
}
