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

#include <string>

#include <guichan/mouseinput.hpp>

#include "button.h"
#include "gui.h"
#include "inventorywindow.h"
#include "item_amount.h"
#include "itemcontainer.h"
#include "scrollarea.h"
#include "viewport.h"

#include "../inventory.h"
#include "../item.h"
#include "../localplayer.h"

#include "../resources/iteminfo.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"
#include "../utils/tostring.h"

InventoryWindow::InventoryWindow():
    Window(_("Inventory"))
{
    setWindowName("Inventory");
    setResizable(true);
    setCloseButton(true);
    setMinWidth(240);
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

    mItemName = _("Name:");
    mItemNameLabel = new TextBox();
    mItemDescription = _("Description:");
    mItemDescriptionLabel = new TextBox();
    mItemEffect = _("Effect:");
    mItemEffectLabel = new TextBox();
    mWeight = _("Weight: ") + mTotalWeight + " g / " +
              mMaxWeight + _(" g Slots: ") + 
              toString(player_node->getInventory()->getNumberOfSlotsUsed()) + 
              "/" + toString(player_node->getInventory()->getInventorySize());
    mWeightLabel = new TextBox();
    mWeightLabel->setPosition(8, 8);
    mItemEffectLabel = new TextBox();

    draw();

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

    if ((mMaxWeight != toString(player_node->mMaxWeight)) ||
         mTotalWeight != toString(player_node->mTotalWeight))
    {
        mTotalWeight = toString(player_node->mTotalWeight);
        mMaxWeight = toString(player_node->mMaxWeight);

        // Adjust widgets
        mWeight = _("Weight: ") + mTotalWeight + " g / " +
                  mMaxWeight + _(" g  Slots: ") + 
                 toString(player_node->getInventory()->getNumberOfSlotsUsed()) + 
                 "/" + toString(player_node->getInventory()->getInventorySize());

        draw();
    }
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
        mItemName = _("Name:");
        mItemNameLabel->setTextWrapped(mItemName);
        mItemEffect = _("Effect:");
        mItemEffectLabel->setTextWrapped(mItemEffect);
        mItemDescription = _("Description:");
        mItemDescriptionLabel->setTextWrapped(mItemDescription);
    }
    else
    {
        const ItemInfo& itemInfo = item->getInfo();
        mItemName = _("Name: ") + itemInfo.getName();
        mItemNameLabel->setTextWrapped(mItemName);
        mItemEffect = _("Effect: ") + itemInfo.getEffect();
        mItemEffectLabel->setTextWrapped(mItemEffect);
        mItemDescription = _("Description: ") + itemInfo.getDescription();
        mItemDescriptionLabel->setTextWrapped(mItemDescription);
    }

    draw();
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

void InventoryWindow::draw()
{
    const gcn::Rectangle &area = getChildrenArea();
    const int width = area.width;
    const int height = area.height;

    // Update weight information
    mWeightLabel->setTextWrapped(mWeight);
    mWeightLabel->setMinWidth(width - 16);

    mUseButton->setPosition(8, height - 8 - mUseButton->getHeight());
    mDropButton->setPosition(8 + mUseButton->getWidth() + 5,
            mUseButton->getY());

    mItemNameLabel->setMinWidth(width - 16);
    mItemNameLabel->setTextWrapped(mItemName);
    mItemNameLabel->setDimension(gcn::Rectangle(8,
            mUseButton->getY() - 5 - (mItemNameLabel->getNumberOfRows()*15),
            width - 16,
            (mItemNameLabel->getNumberOfRows()*15)));
    mItemEffectLabel->setMinWidth(width - 16);
    mItemEffectLabel->setTextWrapped(mItemEffect);
    mItemEffectLabel->setDimension(gcn::Rectangle(8,
            mItemNameLabel->getY() - 5 - (mItemEffectLabel->getNumberOfRows()*15),
            width - 16,
            (mItemEffectLabel->getNumberOfRows()*15)));
    mItemDescriptionLabel->setMinWidth(width - 16);
    mItemDescriptionLabel->setTextWrapped(mItemDescription);
    mItemDescriptionLabel->setDimension(gcn::Rectangle(8,
            mItemEffectLabel->getY() - 5 - (mItemDescriptionLabel->getNumberOfRows()*15),
            width - 16,
            (mItemDescriptionLabel->getNumberOfRows()*15)));

    mInvenScroll->setSize(width - 16,
            mItemDescriptionLabel->getY() - (mWeightLabel->getNumberOfRows()*15) - 18);
    mInvenScroll->setPosition(8, (mWeightLabel->getNumberOfRows()*15) + 10);

    setMinHeight(130 + (mWeightLabel->getNumberOfRows()*15) + 
                       (mItemDescriptionLabel->getNumberOfRows()*15) +
                       (mItemEffectLabel->getNumberOfRows()*15) +
                       (mItemNameLabel->getNumberOfRows()*15));
}

void InventoryWindow::widgetResized(const gcn::Event &event)
{
    Window::widgetResized(event);
    draw();
}

void InventoryWindow::updateButtons()
{
    const Item *selectedItem = mItems->getSelectedItem();

    if (selectedItem && selectedItem->isEquipment())
    {
        if (selectedItem->isEquipped()) {
            mUseButton->setCaption(_("Unequip"));
        }
        else {
            mUseButton->setCaption(_("Equip"));
        }
    }
    else {
        mUseButton->setCaption(_("Use"));
    }

    mUseButton->setEnabled(selectedItem != 0);
    mDropButton->setEnabled(selectedItem != 0);
}

Item* InventoryWindow::getSelectedItem() const
{
    return mItems->getSelectedItem();
}
