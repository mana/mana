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

#include "../graphics.h"
#include "trade.h"
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
    setContentSize(322, 130);
    
    addButton = new Button("Add");
    okButton = new Button("Ok");
    cancelButton = new Button("Cancel");
    tradeButton = new Button("Trade");
    
    addButton->setPosition(2, 105);
    okButton->setPosition(30, 105);
    cancelButton->setPosition(270, 105);
    tradeButton->setPosition(230,105);
    
    addButton->setEventId("add");
    okButton->setEventId("ok");
    cancelButton->setEventId("cancel");
    tradeButton->setEventId("trade");
    
    addButton->addActionListener(this);
    okButton->addActionListener(this);
    cancelButton->addActionListener(this);
    tradeButton->addActionListener(this);

    tradeButton->setEnabled(false);
    
    add(addButton);
    add(okButton);
    add(cancelButton);
    add(tradeButton);
    
    nameLabel = new gcn::Label("Other one");
    nameLabel->setPosition(2, 45);
   
    my_items = new ItemContainer();
    trade_items = new ItemContainer();
   
    my_items->setSize(200, 40);
    trade_items->setSize(200,40);
    
    my_items->setPosition(2, 2);
    trade_items->setPosition(2, 60);
    
    myScroll = new ScrollArea(my_items);
    tradeScroll = new ScrollArea(trade_items);

    myScroll->setPosition(4, 4);
    tradeScroll->setPosition(4, 62);
    
    add(nameLabel);
    add(myScroll);
    add(tradeScroll);
}

TradeWindow::~TradeWindow()
{
    delete addButton;
    delete okButton;
    delete cancelButton;
    delete tradeButton;
    delete nameLabel;
}

void TradeWindow::draw(gcn::Graphics *graphics)
{
    // Draw window graphics
    Window::draw(graphics);
}


int TradeWindow::addItem(int index, int id, bool own, int quantity,
        bool equipment) {
    if (own) {
        my_items->addItem(index, id, quantity, equipment);
    } else {
        trade_items->addItem(index, id, quantity, equipment);
    }
        return 0;
}

int TradeWindow::removeItem(int id, bool own) {
    if (own) {
        my_items->removeItem(id);
    } else {
        trade_items->removeItem(id);
    }
    return 0;
}

int TradeWindow::changeQuantity(int index, bool own, int quantity) {
    if (own) {
        my_items->changeQuantity(index, quantity);
    } else {
        trade_items->changeQuantity(index, quantity);
    }
        return 0;
}

int TradeWindow::increaseQuantity(int index, bool own, int quantity) {
    if (own) {
        my_items->increaseQuantity(index, quantity);
    } else {
        trade_items->increaseQuantity(index, quantity);
    }
    return 0;
}

int TradeWindow::reset() {
    my_items->resetItems();
    trade_items->resetItems();
    tradeButton->setEnabled(false);
    okButton->setEnabled(true);
    ok_other = false;
    ok_me = false;
    return 0;
}

int TradeWindow::setTradeButton(bool enabled) {
    tradeButton->setEnabled(enabled);
    return 0;
}

int TradeWindow::receivedOk(bool own) {
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
    return 0;
}

void TradeWindow::action(const std::string &eventId)
{
    
    if (eventId == "add") {
        // This is still kinda buggy, when ok is clicked, will need to play
        // RO a bit to review its trade behaviour
        if (inventoryWindow->items->getIndex() >= 0 &&
                inventoryWindow->items->getIndex() <= INVENTORY_SIZE) {
            if (tradeWindow->my_items->getFreeSlot() >= 0) {

                WFIFOW(0) = net_w_value(0x00e8);
                WFIFOW(2) = net_w_value(inventoryWindow->items->getIndex());
                WFIFOL(4) = net_l_value(inventoryWindow->items->getQuantity());
                WFIFOSET(8);
                //chatWindow->chat_log("add packet sent", BY_SERVER);
                while ((out_size > 0)) flush();
                              
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
