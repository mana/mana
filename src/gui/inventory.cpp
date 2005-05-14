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

#include "../main.h"
#include "../graphics.h"
#include "inventory.h"
#include "../resources/resourcemanager.h"
#include "../resources/image.h"
#include "button.h"
#include "scrollarea.h"
#include "../being.h"
#include "../engine.h"
#include "item_amount.h"
#include <string>

InventoryWindow::InventoryWindow():
    Window("Inventory")
{
    setContentSize(322, 150);
    useButton = new Button("Use");
    dropButton = new Button("Drop");

    items = new ItemContainer();
    items->setPosition(2, 2);

    invenScroll = new ScrollArea(items);
    invenScroll->setPosition(8, 8);

    useButton->setEventId("use");
    dropButton->setEventId("drop");
    useButton->addActionListener(this);
    dropButton->addActionListener(this);

    itemNameLabel = new gcn::Label("Name:");
    itemDescriptionLabel = new gcn::Label("Description:");


    add(useButton);
    add(dropButton);
    add(invenScroll);
    add(itemNameLabel);
    add(itemDescriptionLabel);

    setResizeable(true);
    setMinWidth(240);
    setMinHeight(150);

    updateWidgets();
    useButton->setSize(48, useButton->getHeight());

}

InventoryWindow::~InventoryWindow()
{
    delete useButton;
    delete dropButton;
    delete itemNameLabel;
    delete itemDescriptionLabel;
}

void InventoryWindow::draw(gcn::Graphics *graphics)
{
    // Draw window graphics
    Window::draw(graphics);
}


int InventoryWindow::addItem(int index, int id, int quantity, bool equipment) {
    items->addItem(index, id, quantity, equipment);
    return 0;
}

int InventoryWindow::removeItem(int id) {
    items->removeItem(id);
    return 0;
}

int InventoryWindow::changeQuantity(int index, int quantity) {
    //items[index].quantity = quantity;
    items->changeQuantity(index, quantity);
    return 0;
}

int InventoryWindow::increaseQuantity(int index, int quantity) {
    //items[index].quantity += quantity;
    items->increaseQuantity(index, quantity);
    return 0;
}

int InventoryWindow::useItem(int index, int id) {
    WFIFOW(0) = net_w_value(0x00a7);
    WFIFOW(2) = net_w_value(index);
    WFIFOL(4) = net_l_value(id);
    // Note: id is dest of item, usually player_node->account_ID ??
    WFIFOSET(8);
    while ((out_size > 0)) flush();
    return 0;
}

int InventoryWindow::dropItem(int index, int quantity) {
    // TODO: Fix wrong coordinates of drops, serverside?
    WFIFOW(0) = net_w_value(0x00a2);
    WFIFOW(2) = net_w_value(index);
    WFIFOW(4) = net_w_value(quantity);
    WFIFOSET(6);
    while ((out_size > 0)) flush();
    return 0;
}

void InventoryWindow::equipItem(int index) {
    WFIFOW(0) = net_w_value(0x00a9);
    WFIFOW(2) = net_w_value(index);
    WFIFOW(4) = net_w_value(0);
    WFIFOSET(6);
    while ((out_size > 0)) flush();
}

void InventoryWindow::unequipItem(int index) {
    WFIFOW(0) = net_w_value(0x00ab);
    WFIFOW(2) = net_w_value(index);
    WFIFOSET(4);
    while ((out_size > 0)) flush();
}

void InventoryWindow::action(const std::string &eventId)
{
    //if(selectedItem >= 0 && selectedItem <= INVENTORY_SIZE) {
    if (items->getIndex() != -1) {
        if (eventId == "use") {
            if(items->isEquipment(items->getIndex())) {
                if(items->isEquipped(items->getIndex())) {
                    unequipItem(items->getIndex());
                }
                else {
                    equipItem(items->getIndex());
                }
            }
            else {
                useItem(items->getIndex(), items->getId());
            }                
        }
        else if (eventId == "drop") {
            itemAmountWindow->setUsage(AMOUNT_ITEM_DROP);
            itemAmountWindow->setVisible(true);
            itemAmountWindow->requestMoveToTop();
        }

        updateUseButton();
    }
}

void InventoryWindow::mouseClick(int x, int y, int button, int count)
{
    Window::mouseClick(x, y, button, count);
    
    updateUseButton();

    if (items->getIndex() != -1)
    {
        // Show Name and Description
        std::string SomeText;
        SomeText = "Name: " + itemDb->getItemInfo(items->getId())->getName();
        itemNameLabel->setCaption(SomeText);
        itemNameLabel->adjustSize();
        SomeText = "Description: " + itemDb->getItemInfo(items->getId())->getDescription();
        itemDescriptionLabel->setCaption(SomeText);
        itemDescriptionLabel->adjustSize();
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
    // Resize widgets
    useButton->setPosition(8, getHeight() - 24);
    dropButton->setPosition(48 + 16, getHeight() - 24);
    items->setSize(getWidth() - 24 - 12 - 1,
            (INVENTORY_SIZE * 24) / (getWidth() / 24) - 1);
    invenScroll->setSize(getWidth() - 16, getHeight() - 72);

    itemNameLabel->setPosition(8, invenScroll->getY() + invenScroll->getHeight() + 4);
    itemDescriptionLabel->setPosition(8,
            itemNameLabel->getY() + itemNameLabel->getHeight() + 4);

    setContentSize(getWidth(), getHeight());
}

void InventoryWindow::updateUseButton()
{
    if (items->getIndex() != -1 && items->isEquipment(items->getIndex()))
    {
        if (items->isEquipped(items->getIndex())) {
            useButton->setCaption("Unequip");
        }
        else {
            useButton->setCaption("Equip");
        }
    }
    else {
        useButton ->setCaption("Use");
    }
    
    useButton->setEnabled(items->getIndex() != -1);
}
