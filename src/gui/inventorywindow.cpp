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

#include "gui/inventorywindow.h"

#include "gui/itemamount.h"
#include "gui/itemcontainer.h"
#include "gui/sdlinput.h"
#include "gui/viewport.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/scrollarea.h"

#include "inventory.h"
#include "item.h"
#include "localplayer.h"
#include "units.h"

#include "net/inventoryhandler.h"
#include "net/net.h"

#include "resources/iteminfo.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>
#include <guichan/mouseinput.hpp>

#include <string>

InventoryWindow::InventoryWindow(int invSize):
    Window(_("Inventory")),
    mMaxSlots(invSize),
    mSplit(false),
    mItemDesc(false)
{
    setWindowName("Inventory");
    setResizable(true);
    setCloseButton(true);
    setSaveVisible(true);

    setDefaultSize(387, 307, ImageRect::CENTER);
    setMinWidth(316);
    setMinHeight(179);
    addKeyListener(this);

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
    mSplitButton = new Button(_("Split"), "split", this);
    mItems = new ItemContainer(player_node->getInventory());
    mItems->addSelectionListener(this);

    gcn::ScrollArea *invenScroll = new ScrollArea(mItems);
    invenScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mTotalWeight = -1;
    mMaxWeight = -1;
    mUsedSlots = -1;

    mSlotsLabel = new Label(_("Slots:"));
    mWeightLabel = new Label(_("Weight:"));

    mSlotsBar = new ProgressBar(1.0f, 100, 20, 225, 200, 25);
    mWeightBar = new ProgressBar(1.0f, 100, 20, 0, 0, 255);

    place(0, 0, mWeightLabel).setPadding(3);
    place(1, 0, mWeightBar, 3);
    place(4, 0, mSlotsLabel).setPadding(3);
    place(5, 0, mSlotsBar, 2);
    place(0, 1, invenScroll, 7).setPadding(3);
    place(0, 2, mUseButton);
    place(1, 2, mDropButton);
    place(2, 2, mSplitButton);

    Layout &layout = getLayout();
    layout.setRowHeight(1, Layout::AUTO_SET);

    loadWindowState();
}

InventoryWindow::~InventoryWindow()
{
}

void InventoryWindow::logic()
{
    if (!isVisible())
        return;

    Window::logic();

    // It would be nicer if this update could be event based, needs some
    // redesign of InventoryWindow and ItemContainer probably.
    updateButtons();

    const int usedSlots = player_node->getInventory()->getNumberOfSlotsUsed();

    if (mMaxWeight != player_node->getMaxWeight() ||
        mTotalWeight != player_node->getTotalWeight() ||
        mUsedSlots != usedSlots)
    {
        mTotalWeight = player_node->getTotalWeight();
        mMaxWeight = player_node->getMaxWeight();
        mUsedSlots = usedSlots;

        // Weight Bar coloration
        if (mTotalWeight < (mMaxWeight / 3))
            mWeightBar->setColor(0, 0, 255); // Blue
        else if (mTotalWeight < ((mMaxWeight * 2) / 3))
            mWeightBar->setColor(255, 255, 0); // Yellow
        else
            mWeightBar->setColor(255, 0, 0); // Red

        // Adjust progress bars
        mSlotsBar->setProgress((float) mUsedSlots / mMaxSlots);
        mWeightBar->setProgress((float) mTotalWeight / mMaxWeight);

        mSlotsBar->setText(strprintf("%d/%d", mUsedSlots, mMaxSlots));
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
        if (item->isEquipment()) {
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
        if (item->getQuantity() > 1) {
            // Choose amount of items to drop
            new ItemAmountWindow(ItemAmountWindow::ItemDrop, this, item);
        }
        else {
            player_node->dropItem(item, 1);
        }
    }
    else if (event.getId() == "split")
    {
        if (item && !item->isEquipment() && item->getQuantity() > 1) {
            new ItemAmountWindow(ItemAmountWindow::ItemSplit, this, item,
                                 (item->getQuantity() - 1));
        }
    }
}

Item* InventoryWindow::getSelectedItem() const
{
    return mItems->getSelectedItem();
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

void InventoryWindow::keyPressed(gcn::KeyEvent &event)
{
    switch (event.getKey().getValue())
    {
        case Key::LEFT_SHIFT:
        case Key::RIGHT_SHIFT:
            mSplit = true;
            break;
    }
}

void InventoryWindow::keyReleased(gcn::KeyEvent &event)
{
    switch (event.getKey().getValue())
    {
        case Key::LEFT_SHIFT:
        case Key::RIGHT_SHIFT:
            mSplit = false;
            break;
    }
}

void InventoryWindow::valueChanged(const gcn::SelectionEvent &event)
{
    if (mSplit)
    {
        Item *item = mItems->getSelectedItem();

        if (item && !item->isEquipment() && item->getQuantity() > 1)
        {
            mSplit = false;
            new ItemAmountWindow(ItemAmountWindow::ItemSplit, this, item,
                                 (item->getQuantity() - 1));
        }
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

    if (Net::getInventoryHandler()->canSplit(selectedItem))
    {
        mSplitButton->setEnabled(true);
    }
    else {
        mSplitButton->setEnabled(false);
    }
}
