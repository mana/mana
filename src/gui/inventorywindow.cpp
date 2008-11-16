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

#include "inventorywindow.h"

#include <string>

#include <guichan/mouseinput.hpp>

#include <guichan/widgets/label.hpp>
#include <guichan/widgets/checkbox.hpp>
#include <guichan/widgets/textbox.hpp>

#include "button.h"
#include "gui.h"
#include "item_amount.h"
#include "itemcontainer.h"
#include "itempopup.h"
#include "scrollarea.h"
#include "sdlinput.h"
#include "viewport.h"

#include "widgets/layout.h"

#include "../item.h"
#include "../localplayer.h"
#include "../log.h"

#include "../resources/iteminfo.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"

InventoryWindow::InventoryWindow():
    Window(_("Inventory")),
    mSplit(false)
{
    setResizable(false);
    setCloseButton(true);
    // LEEOR/TODO: Since this window is not resizable, do we really need to set these
    // values or can we drop them?
    setMinWidth(375);
    setMinHeight(283);
    // If you adjust these defaults, don't forget to adjust the trade window's.
    setDefaultSize(115, 25, 375, 283);
    addKeyListener(this);

    mUseButton = new Button(_("Use"), "use", this);
    mDropButton = new Button(_("Drop"), "drop", this);
    mSplitButton = new Button(_("Split"), "split", this);

    mItems = new ItemContainer(player_node->mInventory, 10, 5);
    mItems->addSelectionListener(this);

    mInvenScroll = new ScrollArea(mItems);

    place(0, 0, mInvenScroll, 100).setPadding(3);
    place(0, 1, mUseButton);
    place(1, 1, mDropButton);
    place(2, 1, mSplitButton);
    Layout &layout = getLayout();
    layout.setColWidth(0, 48);
    layout.setColWidth(1, 48);
    layout.setColWidth(2, 48);
    layout.setRowHeight(0, Layout::AUTO_SET);

    loadWindowState("Inventory");
}

void InventoryWindow::logic()
{
    Window::logic();

    // It would be nicer if this update could be event based, needs some
    // redesign of InventoryWindow and ItemContainer probably.
    updateButtons();

    // Update weight information
    // mWeightLabel->setCaption(strprintf(_("Total Weight: %d - Maximum Weight: %d"), player_node->getTotalWeight(), player_node->getMaxWeight()));
}

void InventoryWindow::action(const gcn::ActionEvent &event)
{
    Item *item = mItems->getItem();
    if (!item) {
        return;
    }

    if (event.getId() == "use")
    {
        if (item->isEquipment()) {
            player_node->equipItem(item);
        }
        else {
            player_node->useItem(item->getInvIndex());
        }
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

void InventoryWindow::valueChanged(const gcn::SelectionEvent &event)
{
    Item *item = mItems->getItem();

    if (mSplit)
    {
        if (item && !item->isEquipment() && item->getQuantity() > 1)
        {
            mSplit = false;
            new ItemAmountWindow(AMOUNT_ITEM_SPLIT, this, item, (item->getQuantity() - 1));
        }
    }
}

void InventoryWindow::mouseClicked(gcn::MouseEvent &event)
{
    Window::mouseClicked(event);

    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Item *item = mItems->getItem();

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
    Item *item = mItems->getItem();

    if (item && item->isEquipment()) {
        mUseButton->setCaption(_("Equip"));
    }
    else {
        mUseButton->setCaption(_("Use"));
    }
    mUseButton->setEnabled(!!item);
    mDropButton->setEnabled(!!item);
    if (item && !item->isEquipment() && item->getQuantity() > 1) {
        mSplitButton->setEnabled(true);
    }
    else {
        mSplitButton->setEnabled(false);
    }
}

Item* InventoryWindow::getItem()
{
    return mItems->getItem();
}

void InventoryWindow::keyPressed(gcn::KeyEvent &event)
{
    switch (event.getKey().getValue())
    {
        case Key::LEFT_SHIFT:
        case Key::RIGHT_SHIFT:
            mSplit = true;
    }
}

void InventoryWindow::keyReleased(gcn::KeyEvent &event)
{
    switch (event.getKey().getValue())
    {
        case Key::LEFT_SHIFT:
        case Key::RIGHT_SHIFT:
            mSplit = false;
    }
}
