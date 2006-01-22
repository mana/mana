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

#include "sell.h"

#include <cassert>
#include <sstream>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "listbox.h"
#include "scrollarea.h"
#include "shop.h"
#include "slider.h"

#include "../item.h"
#include "../npc.h"

#include "../resources/iteminfo.h"
#include "../resources/itemmanager.h"

#include "../net/messageout.h"
#include "../net/protocol.h"


SellDialog::SellDialog(Network *network):
    Window("Sell"),
    mNetwork(network),
    m_maxItems(0), m_amountItems(0)
{
    itemList = new ListBox(this);
    scrollArea = new ScrollArea(itemList);
    slider = new Slider(1.0);
    quantityLabel = new gcn::Label("0");
    moneyLabel = new gcn::Label("Price: 0");
    itemDescLabel = new gcn::Label("Description:");
    itemEffectLabel = new gcn::Label("Effect:");
    increaseButton = new Button("+");
    decreaseButton = new Button("-");
    sellButton = new Button("Sell");
    quitButton = new Button("Quit");
    sellButton->setEnabled(false);

    setContentSize(260, 210);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    scrollArea->setDimension(gcn::Rectangle(5, 5, 250, 110));
    itemList->setDimension(gcn::Rectangle(5, 5, 238, 110));

    slider->setDimension(gcn::Rectangle(5, 120, 200, 10));
    slider->setEnabled(false);

    quantityLabel->setPosition(215, 120);

    increaseButton->setPosition(40, 186);
    increaseButton->setSize(20, 20);
    increaseButton->setEnabled(false);

    decreaseButton->setPosition(10, 186);
    decreaseButton->setSize(20, 20);
    decreaseButton->setEnabled(false);

    moneyLabel->setPosition(5, 130);
    itemEffectLabel->setDimension(gcn::Rectangle(5, 150, 240, 14));
    itemDescLabel->setDimension(gcn::Rectangle(5, 169, 240, 14));

    sellButton->setPosition(175, 186);
    sellButton->setEnabled(false);

    quitButton->setPosition(208, 186);

    itemList->setEventId("item");
    slider->setEventId("slider");
    increaseButton->setEventId("+");
    decreaseButton->setEventId("-");
    sellButton->setEventId("sell");
    quitButton->setEventId("quit");

    itemList->addActionListener(this);
    slider->addActionListener(this);
    increaseButton->addActionListener(this);
    decreaseButton->addActionListener(this);
    sellButton->addActionListener(this);
    quitButton->addActionListener(this);

    add(scrollArea);
    add(slider);
    add(quantityLabel);
    add(moneyLabel);
    add(itemEffectLabel);
    add(itemDescLabel);
    add(increaseButton);
    add(decreaseButton);
    add(sellButton);
    add(quitButton);

    setLocationRelativeTo(getParent());
}

void SellDialog::reset()
{
    shopInventory.clear();
    slider->setValue(0.0);
    m_amountItems = 0;

    quantityLabel->setCaption("0");
    quantityLabel->adjustSize();
    moneyLabel->setCaption("Price: 0");
    moneyLabel->adjustSize();
    itemDescLabel->setCaption("");
    itemEffectLabel->setCaption("");

    // Reset Previous Selected Items to prevent failing asserts
    itemList->setSelected(-1);
    increaseButton->setEnabled(false);
    decreaseButton->setEnabled(false);
}

void SellDialog::addItem(Item *item, int price)
{
    if (!item)
        return;

    ITEM_SHOP item_shop;
    std::stringstream ss;

    ss << item->getInfo()->getName() << " " << price << " GP";

    item_shop.name = ss.str();
    item_shop.price = price;
    item_shop.index = item->getInvIndex();
    item_shop.id = item->getId();
    item_shop.quantity = item->getQuantity();

    shopInventory.push_back(item_shop);
    itemList->adjustSize();
}

void SellDialog::action(const std::string& eventId)
{
    int selectedItem = itemList->getSelected();

    if (eventId == "item") {
        m_amountItems = 0;
        slider->setValue(0);
        decreaseButton->setEnabled(false);
        sellButton->setEnabled(false);

        quantityLabel->setCaption("0");
        quantityLabel->adjustSize();
        moneyLabel->setCaption("Price: 0");
        moneyLabel->adjustSize();

        if (selectedItem > -1) {
            slider->setEnabled(true);
            increaseButton->setEnabled(true);
            m_maxItems = shopInventory[selectedItem].quantity;
        } else {
            slider->setEnabled(false);
            increaseButton->setEnabled(false);
        }
    }
    else if (eventId == "quit")
    {
        setVisible(false);
        current_npc = 0;
    }

    // The following actions require a valid item selection
    if (selectedItem == -1 || selectedItem >= int(shopInventory.size())) {
        return;
    }

    bool updateButtonsAndLabels = false;

    if (eventId == "slider") {
        m_amountItems = (int)(slider->getValue() * m_maxItems);

        updateButtonsAndLabels = true;
    }
    else if (eventId == "+") {
        assert(m_amountItems < m_maxItems);
        m_amountItems++;
        slider->setValue(double(m_amountItems)/double(m_maxItems));

        updateButtonsAndLabels = true;
    }
    else if (eventId == "-") {
        assert(m_amountItems > 0);
        m_amountItems--;

        slider->setValue(double(m_amountItems)/double(m_maxItems));

        updateButtonsAndLabels = true;
    }
    else if (eventId == "sell") {
        // Attempt sell
        assert(m_amountItems > 0 && m_amountItems <= m_maxItems);
        
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(CMSG_NPC_SELL_REQUEST);
        outMsg.writeInt16(8);
        outMsg.writeInt16(shopInventory[selectedItem].index);
        outMsg.writeInt16(m_amountItems);

        m_maxItems -= m_amountItems;
        m_amountItems = 0;
        slider->setValue(0);
        slider->setEnabled(m_maxItems != 0);

        // All were sold
        if (!m_maxItems) {
            itemList->setSelected(-1);
            shopInventory.erase(shopInventory.begin() + selectedItem);
        }

        updateButtonsAndLabels = true;
    }

    // If anything changed, we need to update the buttons and labels
    if (updateButtonsAndLabels) {
        std::stringstream oss;

        // Update labels
        oss << m_amountItems;
        quantityLabel->setCaption(oss.str());
        quantityLabel->adjustSize();
        oss.str("");
        oss << "Price: " << m_amountItems * shopInventory[selectedItem].price;
        moneyLabel->setCaption(oss.str());
        moneyLabel->adjustSize();

        // Update Buttons
        sellButton->setEnabled(m_amountItems > 0);
        decreaseButton->setEnabled(m_amountItems > 0);
        increaseButton->setEnabled(m_amountItems < m_maxItems);
    }
}

void SellDialog::mouseClick(int x, int y, int button, int count)
{
    Window::mouseClick(x, y, button, count);

//    shopInventory[selectedItem];
    int selectedItem = itemList->getSelected();
    if (selectedItem > -1)
    {
        itemDescLabel->setCaption("Description: " +
                itemDb->getItemInfo(shopInventory[selectedItem].id)->getDescription());
        itemEffectLabel->setCaption("Effect: " +
                itemDb->getItemInfo(shopInventory[selectedItem].id)->getEffect());
    }
}

int SellDialog::getNumberOfElements()
{
    return shopInventory.size();
}

std::string SellDialog::getElementAt(int i)
{
    return shopInventory[i].name;
}
