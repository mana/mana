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
#include "../being.h"
#include "../engine.h"
#include <sstream>

TradeWindow::TradeWindow():
    Window("Trade: You")
{
    setContentSize(322, 130);
    
    addButton = new Button("Add");
    cancelButton = new Button("Cancel");
    addButton->setPosition(2, 105);
    cancelButton->setPosition(200, 105);
    
    addButton->setEventId("add");
    cancelButton->setEventId("cancel");
    addButton->addActionListener(this);
    cancelButton->addActionListener(this);
    
    add(addButton);
    add(cancelButton);
    
    nameLabel = new gcn::Label("Other one");
    nameLabel->setPosition(2, 45);
    
    my_items = new ItemContainer();
    trade_items = new ItemContainer();
   
    my_items->setSize(318, 40);
    trade_items->setSize(318,40);
    
    my_items->setPosition(2, 2);
    trade_items->setPosition(2, 60);
    
    add(nameLabel);
    add(my_items);
    add(trade_items);
}

TradeWindow::~TradeWindow()
{
    delete addButton;
    delete cancelButton;
    delete nameLabel;
}

void TradeWindow::draw(gcn::Graphics *graphics)
{
    // Draw window graphics
    Window::draw(graphics);
}


int TradeWindow::addItem(int index, int id, bool own, int quantity,
        bool equipment) {
    my_items->addItem(index, id, quantity, equipment);
    return 0;
}

int TradeWindow::removeItem(int id, bool own) {
    my_items->removeItem(id);
    return 0;
}

int TradeWindow::changeQuantity(int index, bool own, int quantity) {
    //items[index].quantity = quantity;
    my_items->changeQuantity(index, quantity);
    return 0;
}

int TradeWindow::increaseQuantity(int index, bool own, int quantity) {
    //items[index].quantity += quantity;
    my_items->increaseQuantity(index, quantity);
    return 0;
}

/*int TradeWindow::useItem(int index, int id) {
    WFIFOW(0) = net_w_value(0x00a7);
    WFIFOW(2) = net_w_value(index);
    WFIFOL(4) = net_l_value(id);
    // Note: id is dest of item, usually player_node->account_ID ??
    WFIFOSET(8);
    while ((out_size > 0)) flush();
    return 0;
} */

/*int InventoryWindow::dropItem(int index, int quantity) {
    // TODO: Fix wrong coordinates of drops, serverside?
    WFIFOW(0) = net_w_value(0x00a2);
    WFIFOW(2) = net_w_value(index);
    WFIFOW(4) = net_w_value(quantity);
    WFIFOSET(6);
    while ((out_size > 0)) flush();
    return 0;
}*/

/*void InventoryWindow::equipItem(int index) {
    WFIFOW(0) = net_w_value(0x00a9);
    WFIFOW(2) = net_w_value(index);
    WFIFOW(4) = net_w_value(0);
    WFIFOSET(6);
    while ((out_size > 0)) flush();
}*/

/*void InventoryWindow::unequipItem(int index) {
    WFIFOW(0) = net_w_value(0x00ab);
    WFIFOW(2) = net_w_value(index);
    WFIFOSET(4);
    while ((out_size > 0)) flush();
}*/

void TradeWindow::action(const std::string &eventId)
{
    
    if (eventId == "add") {
        if (inventoryWindow->items->getIndex() >= 0 &&
                inventoryWindow->items->getIndex() <= INVENTORY_SIZE) {
            if (tradeWindow->my_items->getFreeSlot() >= 0) {

                WFIFOW(0) = net_w_value(0x00e8);
                WFIFOW(2) = net_w_value(inventoryWindow->items->getIndex());
                WFIFOL(4) = net_l_value(inventoryWindow->items->getQuantity());
                WFIFOSET(8);
                while ((out_size > 0)) flush();
                
            }
        }
    } else if (eventId == "cancel") {
        WFIFOW(0) = net_w_value(0x00ed);
        WFIFOSET(2);
        while ((out_size > 0)) flush();
    }
    //if(selectedItem >= 0 && selectedItem <= INVENTORY_SIZE) {
    
    //if (my_items->getIndex() != -1) {
       /* if (eventId == "use") {
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
        else*/ /*if (eventId == "drop") {
            itemAmountWindow->resetAmount();
            itemAmountWindow->setVisible(true);
        } */      
    //}
}
