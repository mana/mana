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
#include "inventorywindow.h"
#include "item_amount.h"
#include "itemcontainer.h"
#include "progressbar.h"
#include "scrollarea.h"
#include "viewport.h"

#include "widgets/layout.h"

#include "../inventory.h"
#include "../item.h"
#include "../units.h"

#include "../resources/iteminfo.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"
#include "../utils/tostring.h"

InventoryWindow::InventoryWindow(int invSize):
    Window(_("Inventory")),
    mMaxSlots(invSize),
    mItemDesc(false)
{
    setWindowName("Inventory");
    setResizable(true);
    setCloseButton(true);

    // If you adjust these defaults, don't forget to adjust the trade window's.
    setDefaultSize(115, 25, 375, 300);

    std::string longestUseString = getFont()->getWidth(_("Equip")) >
                                   getFont()->getWidth(_("Use")) ?
                                   _("Equip") : _("Use");

    if (getFont()->getWidth(longestUseString) <
        getFont()->getWidth(_("Unequip")))
    {
        longestUseString = _("Unequip");
    }

    mUseButton = new Button(longestUseString, "use", this);
    mDropButton = new Button(_("Drop"), "drop", this);

    mItems = new ItemContainer(player_node->getInventory(), 2);
    mItems->addSelectionListener(this);

    mInvenScroll = new ScrollArea(mItems);
    mInvenScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mTotalWeight = -1;
    mMaxWeight = -1;
    mUsedSlots = toString(player_node->getInventory()->getNumberOfSlotsUsed());

    mSlotsLabel = new gcn::Label(_("Slots: "));
    mWeightLabel = new gcn::Label(_("Weight: "));

    mSlotsBar = new ProgressBar(1.0f, 100, 20, 225, 200, 25);
    mWeightBar = new ProgressBar(1.0f, 100, 20, 0, 0, 255);

    setMinHeight(130);
    setMinWidth(mWeightLabel->getWidth() + mSlotsLabel->getWidth() + 280);

    place(0, 0, mWeightLabel).setPadding(3);
    place(1, 0, mWeightBar, 3);
    place(4, 0, mSlotsLabel).setPadding(3);
    place(5, 0, mSlotsBar, 2);
    place(0, 1, mInvenScroll, 7, 4);
    place(5, 5, mDropButton);
    place(6, 5, mUseButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, mDropButton->getHeight());

    loadWindowState();
    setLocationRelativeTo(getParent());
}

InventoryWindow::~InventoryWindow()
{
    delete mItems;
}

void InventoryWindow::logic()
{
    Window::logic();

    // It would be nicer if this update could be event based, needs some
    // redesign of InventoryWindow and ItemContainer probably.
    updateButtons();

    if (mMaxWeight != player_node->mMaxWeight ||
         mTotalWeight != player_node->mTotalWeight ||
         mUsedSlots != toString(player_node->getInventory()->getNumberOfSlotsUsed()))
    {
        mTotalWeight = player_node->mTotalWeight;
        mMaxWeight = player_node->mMaxWeight;
        mUsedSlots = toString(player_node->getInventory()->getNumberOfSlotsUsed());

        // Weight Bar coloration
        if (int(player_node->mTotalWeight) < int(player_node->mMaxWeight / 3))
        {
            mWeightBar->setColor(0, 0, 255); // Blue
        }
        else if (int(player_node->mTotalWeight) <
                 int((player_node->mMaxWeight / 3) * 2))
        {
            mWeightBar->setColor(255, 255, 0); // Yellow
        }
        else
        {
            mWeightBar->setColor(255, 0, 0); // Red
        }

        // Adjust progress bars
        mSlotsBar->setProgress((float)
               player_node->getInventory()->getNumberOfSlotsUsed() / mMaxSlots);
        mWeightBar->setProgress((float) player_node->mTotalWeight /
                                        player_node->mMaxWeight);

        mSlotsBar->setText(strprintf("%s/%d", mUsedSlots.c_str(), mMaxSlots));
        mWeightBar->setText(strprintf("%s/%s",
                                    Units::formatWeight(mTotalWeight).c_str(),
                                    Units::formatWeight(mMaxWeight).c_str()));
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
