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
#include "sdlinput.h"
#include "viewport.h"

#include "widgets/layout.h"

#include "../inventory.h"
#include "../item.h"
#include "../localplayer.h"
#include "../log.h"
#include "../units.h"

#include "../resources/iteminfo.h"

#include "../utils/gettext.h"
#include "../utils/stringutils.h"
#include "../utils/strprintf.h"

InventoryWindow::InventoryWindow(int invSize):
    Window(_("Inventory")),
    mMaxSlots(invSize),
    mSplit(false),
    mItemDesc(false)
{
    setWindowName("Inventory");
    setResizable(false);
    setCloseButton(true);
    // LEEOR/TODO: Since this window is not resizable, do we really need to set these
    // values or can we drop them?
    setMinWidth(375);
    setMinHeight(283);
    // If you adjust these defaults, don't forget to adjust the trade window's.
    setDefaultSize(115, 30, 375, 283);
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
#ifdef TMWSERV_SUPPORT
    mSplitButton = new Button(_("Split"), "split", this);
#endif

#ifdef TMWSERV_SUPPORT
    mItems = new ItemContainer(player_node->getInventory(), 10, 5);
#else
    mItems = new ItemContainer(player_node->getInventory(), 10, 5, 2);
#endif
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
    place(0, 1, mInvenScroll, 100).setPadding(3);
    place(0, 2, mUseButton);
    place(1, 2, mDropButton);
#ifdef TMWSERV_SUPPORT
    place(2, 2, mSplitButton);
#endif

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    loadWindowState();
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

    if (mMaxWeight != player_node->getMaxWeight() ||
         mTotalWeight != player_node->getTotalWeight() ||
         mUsedSlots != toString(player_node->getInventory()->getNumberOfSlotsUsed()))
    {
        mTotalWeight = player_node->getTotalWeight();
        mMaxWeight = player_node->getMaxWeight();
        mUsedSlots = toString(player_node->getInventory()->getNumberOfSlotsUsed());

        // Weight Bar coloration
        if (int(player_node->getTotalWeight()) < int(player_node->getMaxWeight() / 3))
        {
            mWeightBar->setColor(0, 0, 255); // Blue
        }
        else if (int(player_node->getTotalWeight()) <
                 int((player_node->getMaxWeight() / 3) * 2))
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
        mWeightBar->setProgress((float) player_node->getTotalWeight() /
                                        player_node->getMaxWeight());

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
#ifdef TMWSERV_SUPPORT
        if (item->isEquipment()) {
            player_node->equipItem(item);
        }
        else {
            player_node->useItem(item->getInvIndex());
        }
#else
        if (item->isEquipment())
        {
            if (item->isEquipped())
                player_node->unequipItem(item);
            else
                player_node->equipItem(item);
        }
        else
            player_node->useItem(item);
#endif
    }
    else if (event.getId() == "drop")
    {
        if (item->getQuantity() > 1) {
            // Choose amount of items to drop
            new ItemAmountWindow(AMOUNT_ITEM_DROP, this, item);
        }
        else {
            player_node->dropItem(item, 1);
        }
        mItems->selectNone();
    }
    else if (event.getId() == "split")
    {
        if (item && !item->isEquipment() && item->getQuantity() > 1) {
            new ItemAmountWindow(AMOUNT_ITEM_SPLIT, this, item,
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

#ifdef TMWSERV_SUPPORT
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
#endif

void InventoryWindow::valueChanged(const gcn::SelectionEvent &event)
{
    if (mSplit)
    {
        Item *item = mItems->getSelectedItem();

        if (item && !item->isEquipment() && item->getQuantity() > 1)
        {
            mSplit = false;
            new ItemAmountWindow(AMOUNT_ITEM_SPLIT, this, item, (item->getQuantity() - 1));
        }
    }
}

void InventoryWindow::updateButtons()
{
    const Item *selectedItem = mItems->getSelectedItem();

    if (selectedItem && selectedItem->isEquipment())
    {
#ifdef EATHENA_SUPPORT
        if (selectedItem->isEquipped())
            mUseButton->setCaption(_("Unequip"));
        else
#endif
            mUseButton->setCaption(_("Equip"));
    }
    else
        mUseButton->setCaption(_("Use"));

    mUseButton->setEnabled(selectedItem != 0);
    mDropButton->setEnabled(selectedItem != 0);

#ifdef TMWSERV_SUPPORT
    if (selectedItem && !selectedItem->isEquipment() &&
        selectedItem->getQuantity() > 1)
    {
        mSplitButton->setEnabled(true);
    }
    else {
        mSplitButton->setEnabled(false);
    }
#endif
}
