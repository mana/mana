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

#include "buy.h"
#include "button.h"
#include "slider.h"
#include "scrollarea.h"
#include "listbox.h"
#include "../game.h"
#include "../net/network.h"

#include <sstream>

BuyDialog::BuyDialog():
    Window("Buy"),
    money(0)
{
    itemList = new ListBox(this);
    scrollArea = new ScrollArea(itemList);
    slider = new Slider(1.0);
    quantityLabel = new gcn::Label("0");
    moneyLabel = new gcn::Label("350 G");
    okButton = new Button("OK");
    cancelButton = new Button("Cancel");
    okButton->setEnabled(false);

    setContentSize(260, 175);
    scrollArea->setDimension(gcn::Rectangle(5, 5, 250, 110));
    itemList->setDimension(gcn::Rectangle(5, 5, 238, 110));
    slider->setDimension(gcn::Rectangle(5, 120, 200, 10));
    quantityLabel->setPosition(215, 120);
    moneyLabel->setPosition(5, 135);
    okButton->setPosition(180, 145);
    cancelButton->setPosition(208, 145);

    itemList->setEventId("item");
    slider->setEventId("slider");
    okButton->setEventId("ok");
    cancelButton->setEventId("cancel");

    itemList->addActionListener(this);
    slider->addActionListener(this);
    okButton->addActionListener(this);
    cancelButton->addActionListener(this);

    add(scrollArea);
    add(slider);
    add(quantityLabel);
    add(moneyLabel);
    add(okButton);
    add(cancelButton);

    setLocationRelativeTo(getParent());
}

BuyDialog::~BuyDialog()
{
    delete cancelButton;
    delete okButton;
    delete moneyLabel;
    delete slider;
    delete itemList;
    delete scrollArea;
}

void BuyDialog::setMoney(int amount)
{
    money = amount;
    std::stringstream ss;
    ss << money << " G";
    moneyLabel->setCaption(ss.str());
    moneyLabel->adjustSize();
}

void BuyDialog::reset()
{
    shopInventory.clear();
    money = 0;
}

void BuyDialog::addItem(short id, int price)
{
    ITEM_SHOP item_shop;

    sprintf(item_shop.name, "%s %i gp",
            itemDb.getItemInfo(id)->getName().c_str(), price);
    item_shop.price = price;
    item_shop.id = id;

    shopInventory.push_back(item_shop);
    itemList->adjustSize();
}

void BuyDialog::action(const std::string& eventId)
{
    int selectedItem = itemList->getSelected();

    if (eventId == "slider" || eventId == "item") {
        if (selectedItem > -1) {
            int maxItems = money / shopInventory[selectedItem].price;
            int numItems = (int)(slider->getValue() * maxItems);
            std::stringstream ss;

            ss << numItems;
            quantityLabel->setCaption(ss.str());
            quantityLabel->adjustSize();

            okButton->setEnabled(numItems > 0);
        }
        else {
            okButton->setEnabled(false);
        }
    }
    else if (eventId == "ok") {
        if (selectedItem > -1) {
            // Attempt purchase
            int maxItems = money / shopInventory[selectedItem].price;
            int amount = (int)(slider->getValue() * maxItems);

            if (amount > 0) {
                WFIFOW(0) = net_w_value(0x00c8);
                WFIFOW(2) = net_w_value(8);
                WFIFOW(4) = net_w_value(amount);
                WFIFOW(6) = net_w_value(shopInventory[selectedItem].id);
                WFIFOSET(8);
            }
        }
        setVisible(false);
        current_npc = 0;
    }
    else if (eventId == "cancel") {
        setVisible(false);
        current_npc = 0;
    }
}

int BuyDialog::getNumberOfElements()
{
    return shopInventory.size();
}

std::string BuyDialog::getElementAt(int i)
{
    return shopInventory[i].name;
}
