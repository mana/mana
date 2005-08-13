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

#include <assert.h>
#include <sstream>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "listbox.h"
#include "scrollarea.h"
#include "shop.h"
#include "slider.h"

#include "../game.h"

#include "../resources/iteminfo.h"
#include "../resources/itemmanager.h"

#include "../net/network.h"


BuyDialog::BuyDialog():
    Window("Buy"),
    m_money(0)
{
    itemList = new ListBox(this);
    scrollArea = new ScrollArea(itemList);
    slider = new Slider(1.0);
    quantityLabel = new gcn::Label("0");
    moneyLabel = new gcn::Label("price : 0 G");
    increaseButton = new Button("+");
    decreaseButton = new Button("-");
    buyButton = new Button("Buy");
    quitButton = new Button("Quit");
    itemNameLabel = new gcn::Label("Name:");
    itemDescLabel = new gcn::Label("Description:");

    setContentSize(260, 198);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    scrollArea->setDimension(gcn::Rectangle(5, 5, 250, 110));
    itemList->setDimension(gcn::Rectangle(5, 5, 238, 110));

    slider->setDimension(gcn::Rectangle(5, 120, 200, 10));
    slider->setEnabled(false);

    quantityLabel->setPosition(215, 120);
    moneyLabel->setPosition(5, 133);

    increaseButton->setPosition(40, 174);
    increaseButton->setSize(20, 20);
    increaseButton->setEnabled(false);

    decreaseButton->setPosition(10, 174);
    decreaseButton->setSize(20, 20);
    decreaseButton->setEnabled(false);

    buyButton->setPosition(180, 174);
    buyButton->setEnabled(false);

    quitButton->setPosition(212, 174);

    itemNameLabel->setDimension(gcn::Rectangle(5, 145, 240, 14));
    itemDescLabel->setDimension(gcn::Rectangle(5, 157, 240, 14));

    itemList->setEventId("item");
    slider->setEventId("slider");
    increaseButton->setEventId("+");
    decreaseButton->setEventId("-");
    buyButton->setEventId("buy");
    quitButton->setEventId("quit");

    itemList->addActionListener(this);
    slider->addActionListener(this);
    increaseButton->addActionListener(this);
    decreaseButton->addActionListener(this);
    buyButton->addActionListener(this);
    quitButton->addActionListener(this);


    add(scrollArea);
    add(slider);
    add(quantityLabel);
    add(moneyLabel);
    add(buyButton);
    add(quitButton);
    add(increaseButton);
    add(decreaseButton);
    add(itemNameLabel);
    add(itemDescLabel);

    setLocationRelativeTo(getParent());

    m_amountItems = 0;
    m_maxItems = 0;
    m_money = 0;
}

BuyDialog::~BuyDialog()
{
    delete increaseButton;
    delete decreaseButton;
    delete quitButton;
    delete buyButton;
    delete moneyLabel;
    delete slider;
    delete itemList;
    delete scrollArea;
    delete itemNameLabel;
    delete itemDescLabel;
}

void BuyDialog::setMoney(int amount)
{
    m_money = amount;
    //std::stringstream ss;
    //ss << m_money << " G";
    //moneyLabel->setCaption(ss.str());
    //moneyLabel->adjustSize();
}

void BuyDialog::reset()
{
    shopInventory.clear();
    m_money = 0;
}

void BuyDialog::addItem(short id, int price)
{
    ITEM_SHOP item_shop;

    sprintf(item_shop.name, "%s %i gp",
            itemDb->getItemInfo(id)->getName().c_str(), price);
    item_shop.price = price;
    item_shop.id = id;

    shopInventory.push_back(item_shop);
    itemList->adjustSize();
}

void BuyDialog::action(const std::string& eventId)
{
    int selectedItem = itemList->getSelected();

    std::stringstream oss;

    if (eventId == "item")
    {
        if (selectedItem > -1)
        {
            slider->setEnabled(true);
            increaseButton->setEnabled(true);
            m_amountItems = 0;
            slider->setValue(0);
            oss << m_amountItems;
            quantityLabel->setCaption(oss.str());
            quantityLabel->adjustSize();
            oss.str("price : 0 GP");
            moneyLabel->setCaption(oss.str());
            moneyLabel->adjustSize();
            decreaseButton->setEnabled(false);
            m_maxItems = m_money / shopInventory[selectedItem].price;
        }
        else
        {
            slider->setValue(0);
            slider->setEnabled(false);
            increaseButton->setEnabled(false);
            decreaseButton->setEnabled(false);
            buyButton->setEnabled(false);
            m_amountItems = 0;
            m_maxItems = 0;
      }
    }
    else if (eventId == "slider" && selectedItem > -1)
    {
        m_amountItems = (int)(slider->getValue() * m_maxItems);

        oss << m_amountItems;
        quantityLabel->setCaption(oss.str());
        quantityLabel->adjustSize();

        oss.str("");
        oss << "price : " << m_amountItems * shopInventory[selectedItem].price << " G";
        moneyLabel->setCaption(oss.str());
        moneyLabel->adjustSize();

        if (m_amountItems > 0)
        {
            buyButton->setEnabled(true);
            decreaseButton->setEnabled(true);
        }
        else
        {
            buyButton->setEnabled(false);
            decreaseButton->setEnabled(false);
        }

        if (m_amountItems == m_maxItems)
        {
            increaseButton->setEnabled(false);
        }
        else
        {
             increaseButton->setEnabled(true);
        }
    }
    else if (eventId == "+" && selectedItem > -1)
    {
        assert(m_amountItems < m_maxItems);
        m_amountItems++;
        slider->setValue(double(m_amountItems)/double(m_maxItems));

        decreaseButton->setEnabled(true);
        buyButton->setEnabled(true);
        if (m_amountItems == m_maxItems)
        {
            increaseButton->setEnabled(false);
        }

        oss << m_amountItems;
        quantityLabel->setCaption(oss.str());
        quantityLabel->adjustSize();

        oss.str("");
        oss << "price : " << m_amountItems * shopInventory[selectedItem].price << " G";
        moneyLabel->setCaption(oss.str());
        moneyLabel->adjustSize();
    }
    else if (eventId == "-" && selectedItem > -1)
    {
        assert(m_amountItems > 0);
        m_amountItems--;

        slider->setValue(double(m_amountItems)/double(m_maxItems));

        increaseButton->setEnabled(true);
        if (m_amountItems == 0)
        {
            decreaseButton->setEnabled(false);
            buyButton->setEnabled(false);
        }

        oss << m_amountItems;
        quantityLabel->setCaption(oss.str());
        quantityLabel->adjustSize();

        oss.str("");
        oss << "price : " << m_amountItems * shopInventory[selectedItem].price << " G";
        moneyLabel->setCaption(oss.str());
        moneyLabel->adjustSize();
    }
    else if (eventId == "buy" && selectedItem > -1)
    {
            // Attempt purchase
        assert(m_amountItems > 0 && m_amountItems <= m_maxItems);
        assert(selectedItem >= 0 && selectedItem < int(shopInventory.size()));

        WFIFOW(0) = net_w_value(0x00c8);
        WFIFOW(2) = net_w_value(8);
        WFIFOW(4) = net_w_value(m_amountItems);
        WFIFOW(6) = net_w_value(shopInventory[selectedItem].id);
        WFIFOSET(8);

        // update money !
        m_money -= m_amountItems * shopInventory[selectedItem].price;

        if (m_amountItems == m_maxItems)
        {
            m_maxItems = 0;
            slider->setEnabled(false);
            increaseButton->setEnabled(false);
        }
        else
        {
            m_maxItems = m_money / shopInventory[selectedItem].price;
        }

        decreaseButton->setEnabled(false);
        buyButton->setEnabled(false);

        m_amountItems = 0;
        slider->setValue(0);
        quantityLabel->setCaption("O");
        quantityLabel->adjustSize();

        moneyLabel->setCaption("price : 0 G");
        moneyLabel->adjustSize();
    }
    else if (eventId == "quit")
    {
        setVisible(false);
        current_npc = 0;
    }
}

void BuyDialog::mouseClick(int x, int y, int button, int count)
{
    Window::mouseClick(x, y, button, count);

//    shopInventory[selectedItem];
    int selectedItem = itemList->getSelected();
    if (selectedItem > -1)
    {
        itemNameLabel->setCaption("Name: " +
                itemDb->getItemInfo(shopInventory[selectedItem].id)->getName());
        itemDescLabel->setCaption("Description: " +
                itemDb->getItemInfo(shopInventory[selectedItem].id)->getDescription());
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
