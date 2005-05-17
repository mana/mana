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
 *  $Id $
 */

#include "../main.h"
#include "../graphics.h"
#include "trade.h"
#include "item_amount.h"
#include "../resources/resourcemanager.h"
#include "../resources/image.h"
#include "button.h"
#include "scrollarea.h"
#include "../being.h"
#include "../engine.h"
#include <sstream>

TradeWindow::TradeWindow():
    Window("Trade: You")
{
    setContentSize(322, 150);
    
    addButton = new Button("Add");
    okButton = new Button("Ok");
    cancelButton = new Button("Cancel");
    tradeButton = new Button("Trade");
    
    myItems = new ItemContainer();
    myItems->setPosition(2, 2);
    
    myScroll = new ScrollArea(myItems);
    myScroll->setPosition(8, 8);
    
    partnerItems = new ItemContainer();
    partnerItems->setPosition(2, 58);

    partnerScroll = new ScrollArea(partnerItems);
    partnerScroll->setPosition(8, 64);
    
    addButton->setEventId("add");
    okButton->setEventId("ok");
    cancelButton->setEventId("cancel");
    tradeButton->setEventId("trade");
    
    addButton->addActionListener(this);
    okButton->addActionListener(this);
    cancelButton->addActionListener(this);
    tradeButton->addActionListener(this);

    tradeButton->setEnabled(false);
    
    itemNameLabel = new gcn::Label("Name:");
    itemDescriptionLabel = new gcn::Label("Description:");
    
    add(myScroll);
    add(partnerScroll);
    add(addButton);
    add(okButton);
    add(cancelButton);
    add(tradeButton);
    add(itemNameLabel);
    add(itemDescriptionLabel);
    
    addButton->setPosition(8, getHeight() - 24);
    okButton->setPosition(48 + 16, getHeight() - 24);
    tradeButton->setPosition(getWidth() - 92, getHeight() - 24);
    cancelButton->setPosition(getWidth() - 52, getHeight() - 24);
    
    myItems->setSize(getWidth() - 24 - 12 - 1,
        (INVENTORY_SIZE * 24) / (getWidth() / 24) - 1);
    myScroll->setSize(getWidth() - 16, (getHeight() - 76) / 2);

    partnerItems->setSize(getWidth() - 24 - 12 - 1,
        (INVENTORY_SIZE * 24) / (getWidth() / 24) - 1);
    partnerScroll->setSize(getWidth() - 16, (getHeight() - 76) / 2);
    
    itemNameLabel->setPosition(8,
        partnerScroll->getY() + partnerScroll->getHeight() + 4);
    itemDescriptionLabel->setPosition(8,
        itemNameLabel->getY() + itemNameLabel->getHeight() + 4);

    setContentSize(getWidth(), getHeight());
}

TradeWindow::~TradeWindow()
{
    delete addButton;
    delete okButton;
    delete cancelButton;
    delete tradeButton;
    delete itemNameLabel;
    delete itemDescriptionLabel;
}

void TradeWindow::draw(gcn::Graphics *graphics)
{
    // Draw window graphics
    Window::draw(graphics);
}


void TradeWindow::addItem(int index, int id, bool own, int quantity,
        bool equipment) {
    if (own) {
        myItems->addItem(index, id, quantity, equipment);
    } else {
        partnerItems->addItem(index, id, quantity, equipment);
    }
}

void TradeWindow::removeItem(int id, bool own) {
    if (own) {
        myItems->removeItem(id);
    } else {
        partnerItems->removeItem(id);
    }
}

void TradeWindow::changeQuantity(int index, bool own, int quantity) {
    if (own) {
        myItems->changeQuantity(index, quantity);
    } else {
        partnerItems->changeQuantity(index, quantity);
    }
}

void TradeWindow::increaseQuantity(int index, bool own, int quantity) {
    if (own) {
        myItems->increaseQuantity(index, quantity);
    } else {
        partnerItems->increaseQuantity(index, quantity);
    }
}

void TradeWindow::reset() {
    myItems->resetItems();
    partnerItems->resetItems();
    tradeButton->setEnabled(false);
    okButton->setEnabled(true);
    ok_other = false;
    ok_me = false;
}

void TradeWindow::setTradeButton(bool enabled) {
    tradeButton->setEnabled(enabled);
}

void TradeWindow::receivedOk(bool own) {
    if (own) {
        ok_me = true;
        if (ok_other) {
            tradeButton->setEnabled(true);
            okButton->setEnabled(false);
        } else {
            tradeButton->setEnabled(false);
            okButton->setEnabled(false);
        }
    } else {
        ok_other = true;
        if (ok_me) {
            tradeButton->setEnabled(true);
            okButton->setEnabled(false);
        } else {
            tradeButton->setEnabled(false);
            okButton->setEnabled(true);
        }
    }
}

void TradeWindow::tradeItem(int index, int quantity)
{
    WFIFOW(0) = net_w_value(0x00e8);
    WFIFOW(2) = net_w_value(index);
    WFIFOL(4) = net_l_value(quantity);
    WFIFOSET(8);
    while ((out_size > 0)) flush();
}

void TradeWindow::mouseClick(int x, int y, int button, int count)
{

    Window::mouseClick(x, y, button, count);
    
    // myItems selected
    if (x >= myScroll->getX() + 3
        && x <= myScroll->getX() + myScroll->getWidth() - 10
        && y >= myScroll->getY() + 16
        && y <= myScroll->getY() + myScroll->getHeight() + 15)
    {
        if (myItems->getIndex() != -1)
        {
            partnerItems->selectNone();
            
            // Show Name and Description
            std::string SomeText;
            SomeText = "Name: " +
                itemDb->getItemInfo(myItems->getId())->getName();
            itemNameLabel->setCaption(SomeText);
            itemNameLabel->adjustSize();
            SomeText = "Description: " +
                itemDb->getItemInfo(myItems->getId())->getDescription();
            itemDescriptionLabel->setCaption(SomeText);
            itemDescriptionLabel->adjustSize();
        }
    // partnerItems selected
    } else if (x >= partnerScroll->getX() + 3
        && x <= partnerScroll->getX() + partnerScroll->getWidth() - 20
        && y >= partnerScroll->getY() + 16
        && y <= partnerScroll->getY() + partnerScroll->getHeight() + 15)
    {
        if (partnerItems->getIndex() != -1)
        {   
            myItems->selectNone();
            
            // Show Name and Description
            std::string SomeText;
            SomeText = "Name: " +
                itemDb->getItemInfo(partnerItems->getId())->getName();
            itemNameLabel->setCaption(SomeText);
            itemNameLabel->adjustSize();
            SomeText = "Description: " +
                itemDb->getItemInfo(partnerItems->getId())->getDescription();
            itemDescriptionLabel->setCaption(SomeText);
            itemDescriptionLabel->adjustSize();
        }
    }
}

void TradeWindow::action(const std::string &eventId)
{
    
    if (eventId == "add") {
        if (inventoryWindow->items->getIndex() >= 0 &&
                inventoryWindow->items->getIndex() <= INVENTORY_SIZE) {
            if (tradeWindow->myItems->getFreeSlot() >= 0) {
                if (inventoryWindow->items->getQuantity() == 1) {
                    tradeItem(inventoryWindow->items->getIndex(), 1);
                }
                else {
                    itemAmountWindow->setUsage(AMOUNT_TRADE_ADD);
                    itemAmountWindow->setVisible(true);
                    itemAmountWindow->requestMoveToTop();
                }
            }
        }
    } else if (eventId == "cancel") {
        WFIFOW(0) = net_w_value(0x00ed);
        WFIFOSET(2);
        while ((out_size > 0)) flush();
    } else if (eventId == "ok") {
        WFIFOW(0) = net_w_value(0x00eb);
        WFIFOSET(2);
        while ((out_size > 0)) flush();
    } else if (eventId == "trade") {
        WFIFOW(0) = net_w_value(0x00ef);
        WFIFOSET(2);
        while ((out_size > 0)) flush();
    }
}
