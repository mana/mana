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
#include <sstream>

#include <guichan/mouseinput.hpp>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "item_amount.h"
#include "itemcontainer.h"
#include "popupmenu.h"
#include "scrollarea.h"

#include "../inventory.h"
#include "../item.h"
#include "../playerinfo.h"

#include "../resources/iteminfo.h"

InventoryWindow::InventoryWindow():
    Window("Inventory")
{
    setContentSize(322, 172);
    useButton = new Button("Use");
    dropButton = new Button("Drop");

    items = new ItemContainer(inventory);
    invenScroll = new ScrollArea(items);
    invenScroll->setPosition(8, 8);
    invenScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    useButton->setEventId("use");
    dropButton->setEventId("drop");
    useButton->addActionListener(this);
    dropButton->addActionListener(this);

    itemNameLabel = new gcn::Label("Name:");
    itemDescriptionLabel = new gcn::Label("Description:");
    itemEffectLabel = new gcn::Label("Effect:");
    weightLabel = new gcn::Label("Total Weight: - Maximum Weight: ");
    weightLabel->setPosition(8, 8);
    invenScroll->setPosition(8,
            weightLabel->getY() + weightLabel->getHeight() + 5);

    add(useButton);
    add(dropButton);
    add(invenScroll);
    add(itemNameLabel);
    add(itemDescriptionLabel);
    add(itemEffectLabel);
    add(weightLabel);

    setResizable(true);
    setMinWidth(240);
    setMinHeight(172);

    updateWidgets();
    useButton->setSize(48, useButton->getHeight());
}

InventoryWindow::~InventoryWindow()
{
    delete useButton;
    delete dropButton;
    delete invenScroll;
    delete items;
    delete itemNameLabel;
    delete itemDescriptionLabel;
    delete itemEffectLabel;
    delete weightLabel;
}

void InventoryWindow::logic()
{
    Window::logic();

    // It would be nicer if this update could be event based, needs some
    // redesign of InventoryWindow and ItemContainer probably.
    updateButtons();

    // Update weight information
    std::stringstream tempstr;
    tempstr << "Total Weight: " << player_info->totalWeight
            << " - Maximum Weight: " << player_info->maxWeight;
    weightLabel->setCaption(tempstr.str());
    weightLabel->adjustSize();
}

void InventoryWindow::action(const std::string &eventId)
{
    Item *item = items->getItem();

    if (!item) {
        return;
    }

    if (eventId == "use") {
        if (item->isEquipment()) {
            if (item->isEquipped()) {
                inventory->unequipItem(item);
            }
            else {
                inventory->equipItem(item);
            }
        }
        else {
            inventory->useItem(item);
        }
    }
    else if (eventId == "drop")
    {
        // Choose amount of items to drop
        new ItemAmountWindow(AMOUNT_ITEM_DROP, this, item);
    }
}

void InventoryWindow::mouseClick(int x, int y, int button, int count)
{
    Window::mouseClick(x, y, button, count);

    Item *item = items->getItem();

    if (!item) {
        return;
    }

    // Show Name and Description
    std::string SomeText;
    SomeText = "Name: " + item->getInfo()->getName();
    itemNameLabel->setCaption(SomeText);
    itemNameLabel->adjustSize();
    SomeText = "Effect: " + item->getInfo()->getEffect();
    itemEffectLabel->setCaption(SomeText);
    itemEffectLabel->adjustSize();
    SomeText = "Description: " + item->getInfo()->getDescription();
    itemDescriptionLabel->setCaption(SomeText);
    itemDescriptionLabel->adjustSize();

    if (button == gcn::MouseInput::RIGHT)
    {
        /*
         * convert relative to the window coordinates to
         * absolute screen coordinates
         */
        int mx = x + getX();
        int my = y + getY();
        popupMenu->showPopup(mx, my, item);
    }
}

void InventoryWindow::mouseMotion(int mx, int my)
{
    int tmpWidth = getWidth(), tmpHeight = getHeight();
    Window::mouseMotion(mx, my);
    if (getWidth() != tmpWidth || getHeight() != tmpHeight) {
        updateWidgets();
    }
}

void InventoryWindow::updateWidgets()
{
    int width = getContent()->getWidth();
    int height = getContent()->getHeight();
    int columns = width / 24;

    if (columns < 1)
    {
        columns = 1;
    }

    // Resize widgets
    useButton->setPosition(8, height - 24);
    dropButton->setPosition(48 + 16, height - 24);
    invenScroll->setSize(width - 16, height - 110);

    itemNameLabel->setPosition(8,
            invenScroll->getY() + invenScroll->getHeight() + 4);
    itemEffectLabel->setPosition(8,
            itemNameLabel->getY() + itemNameLabel->getHeight() + 4);
    itemDescriptionLabel->setPosition(8,
            itemEffectLabel->getY() + itemEffectLabel->getHeight() + 4);
}

void InventoryWindow::updateButtons()
{
    Item *item;

    if ((item = items->getItem()) && item->isEquipment())
    {
        if (item->isEquipped()) {
            useButton->setCaption("Unequip");
        }
        else {
            useButton->setCaption("Equip");
        }
    }
    else {
        useButton ->setCaption("Use");
    }

    useButton->setEnabled(!!item);
    dropButton->setEnabled(!!item);
}

Item* InventoryWindow::getItem()
{
    return items->getItem();
}
