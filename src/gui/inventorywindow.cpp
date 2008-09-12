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
 *  $Id: inventorywindow.cpp 4347 2008-06-12 09:06:01Z b_lindeijer $
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

#include "../utils/tostring.h"

InventoryWindow::InventoryWindow():
    Window("Inventory")
{
    setWindowName("Inventory");
    setResizable(true);
    setCloseButton(true);
    setMinWidth(240);
    setMinHeight(172);
    // If you adjust these defaults, don't forget to adjust the trade window's.
    setDefaultSize(115, 25, 322, 172);

    mUseButton = new Button("Use", "use", this);
    mDropButton = new Button("Drop", "drop", this);

    mItems = new ItemContainer(player_node->getInventory(), 2);
    mItems->addSelectionListener(this);

    mInvenScroll = new ScrollArea(mItems);
    mInvenScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    mItemNameLabel = new gcn::Label("Name:");
    mItemDescriptionLabel = new gcn::Label("Description:");
    mItemEffectLabel = new gcn::Label("Effect:");
    mWeightLabel = new gcn::Label("Total Weight: - Maximum Weight: ");
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

    mUseButton->setSize(60, mUseButton->getHeight());

    loadWindowState();
}

void InventoryWindow::logic()
{
    Window::logic();

    // It would be nicer if this update could be event based, needs some
    // redesign of InventoryWindow and ItemContainer probably.
    updateButtons();

    // Update weight information
    mWeightLabel->setCaption(
        "Total Weight: " + toString(player_node->mTotalWeight) + " - " +
        "Maximum Weight: " + toString(player_node->mMaxWeight));
}

void InventoryWindow::action(const gcn::ActionEvent &event)
{
    Item *item = mItems->getSelectedItem();

    if (!item)
        return;

    if (event.getId() == "use") {
        if (item->isEquipment()) {
            if (item->isEquipped()) {
                player_node->unequipItem(item);
            }
            else {
                player_node->equipItem(item);
            }
        }
        else {
            player_node->useItem(item);
        }
    }
    else if (event.getId() == "drop")
    {
        if (item->getQuantity() == 1) {
            player_node->dropItem(item, 1);
        }
        else {
            // Choose amount of items to drop
            new ItemAmountWindow(AMOUNT_ITEM_DROP, this, item);
        }
    }
}

void InventoryWindow::valueChanged(const gcn::SelectionEvent &event)
{
    const Item *item = mItems->getSelectedItem();

    // Update name, effect and description
    if (!item)
    {
        mItemNameLabel->setCaption("Name:");
        mItemEffectLabel->setCaption("Effect:");
        mItemDescriptionLabel->setCaption("Description:");
    }
    else
    {
        const ItemInfo& itemInfo = item->getInfo();
        std::string SomeText;
        SomeText = "Name: " + itemInfo.getName();
        mItemNameLabel->setCaption(SomeText);
        SomeText = "Effect: " + itemInfo.getEffect();
        mItemEffectLabel->setCaption(SomeText);
        SomeText = "Description: " + itemInfo.getDescription();
        mItemDescriptionLabel->setCaption(SomeText);
    }
}

void InventoryWindow::mouseClicked(gcn::MouseEvent &event)
{
    Window::mouseClicked(event);

    if (event.getButton() == gcn::MouseEvent::RIGHT)
    {
        Item *item = mItems->getSelectedItem();

        if (!item) return;

        /* Convert relative to the window coordinates to absolute screen
         * coordinates.
         */
        const int mx = event.getX() + getX();
        const int my = event.getY() + getY();
        viewport->showPopup(mx, my, item);
    }
}

void InventoryWindow::widgetResized(const gcn::Event &event)
{
    Window::widgetResized(event);

    const gcn::Rectangle &area = getChildrenArea();
    const int width = area.width;
    const int height = area.height;

    // Adjust widgets
    mUseButton->setPosition(8, height - 8 - mUseButton->getHeight());
    mDropButton->setPosition(8 + mUseButton->getWidth() + 5,
            mUseButton->getY());

    mItemNameLabel->setDimension(gcn::Rectangle(8,
            mUseButton->getY() - 5 - mItemNameLabel->getHeight(),
            width - 16,
            mItemNameLabel->getHeight()));
    mItemEffectLabel->setDimension(gcn::Rectangle(8,
            mItemNameLabel->getY() - 5 - mItemEffectLabel->getHeight(),
            width - 16,
            mItemEffectLabel->getHeight()));
    mItemDescriptionLabel->setDimension(gcn::Rectangle(8,
            mItemEffectLabel->getY() - 5 - mItemDescriptionLabel->getHeight(),
            width - 16,
            mItemDescriptionLabel->getHeight()));

    mInvenScroll->setSize(width - 16,
            mItemDescriptionLabel->getY() - mWeightLabel->getHeight() - 18);

    mWeightLabel->setWidth(width - 16);
}

void InventoryWindow::updateButtons()
{
    const Item *selectedItem = mItems->getSelectedItem();

    if (selectedItem && selectedItem->isEquipment())
    {
        if (selectedItem->isEquipped()) {
            mUseButton->setCaption("Unequip");
        }
        else {
            mUseButton->setCaption("Equip");
        }
    }
    else {
        mUseButton->setCaption("Use");
    }

    mUseButton->setEnabled(selectedItem != 0);
    mDropButton->setEnabled(selectedItem != 0);
}

Item* InventoryWindow::getSelectedItem() const
{
    return mItems->getSelectedItem();
}
