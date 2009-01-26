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

#include <string>

#include <guichan/font.hpp>
#include <guichan/mouseinput.hpp>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "gui.h"
#include "inventorywindow.h"
#include "item_amount.h"
#include "itemcontainer.h"
#include "scrollarea.h"
#include "viewport.h"

#include "widgets/layout.h"

#include "../inventory.h"
#include "../item.h"

#include "../resources/iteminfo.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"
#include "../utils/tostring.h"

InventoryWindow::InventoryWindow(int invSize):
    Window(_("Inventory")),
    mSize(invSize),
    mItemDesc(false)
{
    setWindowName(_("Inventory"));
    setResizable(true);
    setCloseButton(true);

    // If you adjust these defaults, don't forget to adjust the trade window's.
    setDefaultSize(115, 25, 322, 200);

    mUseButton = new Button(_("Use"), "use", this);
    mDropButton = new Button(_("Drop"), "drop", this);

    mItems = new ItemContainer(player_node->getInventory(), 2);
    mItems->addSelectionListener(this);

    mInvenScroll = new ScrollArea(mItems);
    mInvenScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mTotalWeight = toString(player_node->mTotalWeight);
    mMaxWeight = toString(player_node->mMaxWeight);

    mWeight = strprintf(_("Weight: %d  g / %d g"),
                              player_node->mTotalWeight,
                              player_node->mMaxWeight) + " " +
              strprintf(_("Slots used: %d / %d"),
                              player_node->getInventory()->getNumberOfSlotsUsed(),
                              mSize);

    mWeightLabel = new gcn::Label(mWeight);

    setMinHeight(130);
    setMinWidth(getFont()->getWidth(mWeight));

    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mInvenScroll, 5, 4);
    place(0, 4, mWeightLabel, 5);
    place(3, 5, mDropButton);
    place(4, 5, mUseButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    loadWindowState();
    setLocationRelativeTo(getParent());
}

void InventoryWindow::logic()
{
    Window::logic();

    // It would be nicer if this update could be event based, needs some
    // redesign of InventoryWindow and ItemContainer probably.
    updateButtons();

    if ((mMaxWeight != toString(player_node->mMaxWeight)) ||
         mTotalWeight != toString(player_node->mTotalWeight))
    {
        mTotalWeight = toString(player_node->mTotalWeight);
        mMaxWeight = toString(player_node->mMaxWeight);

        // Adjust widgets
        mWeight = strprintf(_("Weight: %d  g / %d g"),
                              player_node->mTotalWeight,
                              player_node->mMaxWeight) + " " +
                  strprintf(_("Slots used: %d / %d"),
                              player_node->getInventory()->getNumberOfSlotsUsed(),
                              mSize);

        setMinWidth(getFont()->getWidth(mWeight));
    }
}

void InventoryWindow::action(const gcn::ActionEvent &event)
{
    Item *item = mItems->getSelectedItem();

    if (!item)
        return;

    if (event.getId() == "use")
    {
        if (item->isEquipment())
        {
            if (item->isEquipped())
                player_node->unequipItem(item);
            else
                player_node->equipItem(item);
        }
        else
            player_node->useItem(item);
    }
    else if (event.getId() == "drop")
    {
        if (item->getQuantity() == 1)
            player_node->dropItem(item, 1);
        else
        {
            // Choose amount of items to drop
            new ItemAmountWindow(AMOUNT_ITEM_DROP, this, item);
        }
    }
}

void InventoryWindow::mouseClicked(gcn::MouseEvent &event)
{
    Window::mouseClicked(event);

    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Item *item = mItems->getSelectedItem();

        if (!item)
            return;

        /* Convert relative to the window coordinates to absolute screen
         * coordinates.
         */
        const int mx = event.getX() + getX();
        const int my = event.getY() + getY();
        viewport->showPopup(mx, my, item);
    }
}

void InventoryWindow::updateButtons()
{
    const Item *selectedItem = mItems->getSelectedItem();

    if (selectedItem && selectedItem->isEquipment())
    {
        if (selectedItem->isEquipped())
            mUseButton->setCaption(_("Unequip"));
        else
            mUseButton->setCaption(_("Equip"));
    }
    else
        mUseButton->setCaption(_("Use"));

    mUseButton->setEnabled(selectedItem != 0);
    mDropButton->setEnabled(selectedItem != 0);
}

Item* InventoryWindow::getSelectedItem() const
{
    return mItems->getSelectedItem();
}
