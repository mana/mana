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
#include <guichan/widgets/checkbox.hpp>

#include "button.h"
#include "gui.h"
#include "item_amount.h"
#include "itemcontainer.h"
#include "scrollarea.h"
#include "viewport.h"

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
    setResizable(true);
    setCloseButton(true);
    setMinWidth(240);
    setMinHeight(172);
    // If you adjust these defaults, don't forget to adjust the trade window's.
    setDefaultSize(115, 25, 368, 326);
    addKeyListener(this);

    mUseButton = new Button(_("Use"), "use", this);
    mDropButton = new Button(_("Drop"), "drop", this);
    mSplitButton = new Button(_("Split"), "split", this);

    mUseButton->adjustSize();
    mDropButton->adjustSize();
    mSplitButton->adjustSize();

    mItems = new ItemContainer(player_node->mInventory.get(), 10, 5);
    mItems->addSelectionListener(this);

    mInvenScroll = new ScrollArea(mItems);

    mItemNameLabel = new gcn::Label(strprintf(_("Name: %s"), ""));
    mItemDescriptionLabel = new gcn::Label(
        strprintf(_("Description: %s"), ""));
    mItemEffectLabel = new gcn::Label(strprintf(_("Effect: %s"), ""));
    mWeightLabel = new gcn::Label(
        strprintf(_("Total Weight: %d - Maximum Weight: %d"), 0, 0));
    mWeightLabel->setPosition(8, 8);
    mInvenScroll->setPosition(8,
            mWeightLabel->getY() + mWeightLabel->getHeight() + 5);

    add(mUseButton);
    add(mDropButton);
    add(mSplitButton);
    add(mInvenScroll);
    add(mItemNameLabel);
    add(mItemDescriptionLabel);
    add(mItemEffectLabel);
    add(mWeightLabel);

    if (mUseButton->getWidth() < 48) {
        mUseButton->setWidth(48);
    }
    if (mDropButton->getWidth() < 48) {
        mDropButton->setWidth(48);
    }
    if (mSplitButton->getWidth() < 48) {
        mSplitButton->setWidth(48);
    }

    addWindowListener(this);
    loadWindowState("Inventory");
}

InventoryWindow::~InventoryWindow()
{
    delete mUseButton;
    delete mDropButton;
    delete mSplitButton;
    delete mItems;
    delete mInvenScroll;

    delete mItemNameLabel;
    delete mItemDescriptionLabel;
    delete mItemEffectLabel;
    delete mWeightLabel;
}

void InventoryWindow::logic()
{
    Window::logic();

    // It would be nicer if this update could be event based, needs some
    // redesign of InventoryWindow and ItemContainer probably.
    updateButtons();

    // Update weight information
    mWeightLabel->setCaption(
        strprintf(_("Total Weight: %d - Maximum Weight: %d"),
        player_node->getTotalWeight(), player_node->getMaxWeight()));
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

void InventoryWindow::selectionChanged(const SelectionEvent &event)
{
    Item *item = mItems->getItem();
    ItemInfo const *info = item ? &item->getInfo() : NULL;

    mItemNameLabel->setCaption(strprintf(_("Name: %s"),
        info ? info->getName().c_str() : ""));
    mItemEffectLabel->setCaption(strprintf(_("Effect: %s"),
        info ? info->getEffect().c_str() : ""));
    mItemDescriptionLabel->setCaption(strprintf(_("Description: %s"),
        info ? info->getDescription().c_str() : ""));

    mItemNameLabel->adjustSize();
    mItemEffectLabel->adjustSize();
    mItemDescriptionLabel->adjustSize();

    if (mSplit)
    {
        if (item && !item->isEquipment() && item->getQuantity() > 1) {
            mSplit = false;
            new ItemAmountWindow(AMOUNT_ITEM_SPLIT, this, item,
                (item->getQuantity() - 1));
        }
    }
}

void InventoryWindow::mouseClicked(gcn::MouseEvent &event)
{
    Window::mouseClicked(event);

    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Item *item = mItems->getItem();

        if (!item) {
            return;
        }
        /* Convert relative to the window coordinates to absolute screen
         * coordinates.
         */
        const int mx = event.getX() + getX();
        const int my = event.getY() + getY();
        viewport->showPopup(mx, my, item);
    }
}

void InventoryWindow::windowResized(const WindowEvent &event)
{
    const gcn::Rectangle area = getChildrenArea();

    // Resize widgets
    mUseButton->setPosition(8, area.height - 24);
    mDropButton->setPosition(mUseButton->getWidth() + 16, area.height - 24);
    mSplitButton->setPosition(
        mUseButton->getWidth() + mDropButton->getWidth() + 24,
        area.height - 24);
    mInvenScroll->setSize(area.width - 16, area.height - 110);

    mItemNameLabel->setPosition(8,
        mInvenScroll->getY() + mInvenScroll->getHeight() + 4);
    mItemEffectLabel->setPosition(8,
        mItemNameLabel->getY() + mItemNameLabel->getHeight() + 4);
    mItemDescriptionLabel->setPosition(8,
        mItemEffectLabel->getY() + mItemEffectLabel->getHeight() + 4);
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
        case gcn::Key::LEFT_SHIFT:
        case gcn::Key::RIGHT_SHIFT:
            mSplit = true;
    }
}

void InventoryWindow::keyReleased(gcn::KeyEvent &event)
{
    switch (event.getKey().getValue())
    {
        case gcn::Key::LEFT_SHIFT:
        case gcn::Key::RIGHT_SHIFT:
            mSplit = false;
    }
}
