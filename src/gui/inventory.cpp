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

#include "../playerinfo.h"
#include "inventory.h"
#include "../equipment.h"
#include "button.h"
#include "scrollarea.h"
#include "../net/network.h"
#include "item_amount.h"
#include <string>

InventoryWindow::InventoryWindow():
    Window("Inventory")
{
    setContentSize(322, 160);
    useButton = new Button("Use");
    dropButton = new Button("Drop");

    items = new ItemContainer();
    invenScroll = new ScrollArea(items);
    invenScroll->setPosition(8, 8);
    invenScroll->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);

    useButton->setEventId("use");
    dropButton->setEventId("drop");
    useButton->addActionListener(this);
    dropButton->addActionListener(this);

    itemNameLabel = new gcn::Label("Name:");
    itemDescriptionLabel = new gcn::Label("Description:");
    weightLabel = new gcn::Label("Total Weight: - Maximum Weight: ");
    weightLabel->setPosition(8, 8);
    invenScroll->setPosition(8,
            weightLabel->getY() + weightLabel->getHeight() + 5);

    add(useButton);
    add(dropButton);
    add(invenScroll);
    add(itemNameLabel);
    add(itemDescriptionLabel);
    add(weightLabel);

    setResizable(true);
    setMinWidth(240);
    setMinHeight(150);

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
    delete weightLabel;
}

void InventoryWindow::logic()
{
    Window::logic();

    // It would be nicer if this update could be event based, needs some
    // redesign of InventoryWindow and ItemContainer probably.
    updateButtons();

    // Update weight information
    char tempstr[128];
    sprintf(tempstr, "Total Weight: %2i - Maximum Weight: %2i",
            char_info->totalWeight, char_info->maxWeight);
    weightLabel->setCaption(tempstr);
    weightLabel->adjustSize();
}

int InventoryWindow::useItem(Item *item)
{
    WFIFOW(0) = net_w_value(0x00a7);
    WFIFOW(2) = net_w_value(item->getInvIndex());
    WFIFOL(4) = net_l_value(item->getId());
    // Note: id is dest of item, usually player_node->account_ID ??
    WFIFOSET(8);
    while ((out_size > 0)) flush();
    return 0;
}

int InventoryWindow::dropItem(Item *item, int quantity)
{
    // TODO: Fix wrong coordinates of drops, serverside?
    WFIFOW(0) = net_w_value(0x00a2);
    WFIFOW(2) = net_w_value(item->getInvIndex());
    WFIFOW(4) = net_w_value(quantity);
    WFIFOSET(6);
    while ((out_size > 0)) flush();
    return 0;
}

void InventoryWindow::equipItem(Item *item)
{
    WFIFOW(0) = net_w_value(0x00a9);
    WFIFOW(2) = net_w_value(item->getInvIndex());
    WFIFOW(4) = net_w_value(0);
    WFIFOSET(6);
    while ((out_size > 0)) flush();
}

void InventoryWindow::unequipItem(Item *item)
{
    WFIFOW(0) = net_w_value(0x00ab);
    WFIFOW(2) = net_w_value(item->getInvIndex());
    WFIFOSET(4);
    while ((out_size > 0)) flush();

    // Tidy equipment directly to avoid weapon still shown bug, by instance
    Equipment::getInstance()->removeEquipment(item);
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
                unequipItem(item);
            }
            else {
                equipItem(item);
            }
        }
        else {
            useItem(item);
        }
    }
    else if (eventId == "drop")
    {
        // Choose amount of items to drop
        new ItemAmountWindow(AMOUNT_ITEM_DROP, this);
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
    SomeText = "Description: " + item->getInfo()->getDescription();
    itemDescriptionLabel->setCaption(SomeText);
    itemDescriptionLabel->adjustSize();
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
    invenScroll->setSize(width - 16, height - 90);

    itemNameLabel->setPosition(8,
            invenScroll->getY() + invenScroll->getHeight() + 4);
    itemDescriptionLabel->setPosition(8,
            itemNameLabel->getY() + itemNameLabel->getHeight() + 4);
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
