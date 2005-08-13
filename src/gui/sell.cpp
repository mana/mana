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

#include <assert.h>
#include <sstream>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "listbox.h"
#include "scrollarea.h"
#include "shop.h"
#include "slider.h"

#include "../game.h"
#include "../inventory.h"
#include "../item.h"

#include "../resources/iteminfo.h"

#include "../net/network.h"


SellDialog::SellDialog():
    Window("Sell")
{
    itemList = new ListBox(this);
    scrollArea = new ScrollArea(itemList);
    slider = new Slider(1.0);
    quantityLabel = new gcn::Label("0");
    increaseButton = new Button("+");
    decreaseButton = new Button("-");
    sellButton = new Button("Sell");
    quitButton = new Button("Quit");
    sellButton->setEnabled(false);

    setContentSize(260, 175);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    scrollArea->setDimension(gcn::Rectangle(5, 5, 250, 110));
    itemList->setDimension(gcn::Rectangle(5, 5, 238, 110));

    slider->setDimension(gcn::Rectangle(5, 120, 200, 10));
    slider->setEnabled(false);

    quantityLabel->setPosition(215, 120);

    increaseButton->setPosition(40, 145);
    increaseButton->setSize(20, 20);
    increaseButton->setEnabled(false);

    decreaseButton->setPosition(10, 145);
    decreaseButton->setSize(20, 20);
    decreaseButton->setEnabled(false);

    sellButton->setPosition(175, 145);
    sellButton->setEnabled(false);

    quitButton->setPosition(208, 145);

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
    add(increaseButton);
    add(decreaseButton);
    add(sellButton);
    add(quitButton);

    setLocationRelativeTo(getParent());

    m_maxItems = 0;
    m_amountItems = 0;
}

SellDialog::~SellDialog()
{
    delete increaseButton;
    delete decreaseButton;
    delete quitButton;
    delete sellButton;
    delete slider;
    delete itemList;
    delete scrollArea;
}

void SellDialog::reset()
{
    shopInventory.clear();
    slider->setValue(0.0);
    m_amountItems = 0;
    quantityLabel->setCaption("0");
    quantityLabel->adjustSize();
}

void SellDialog::addItem(short index, int price)
{
    Item *item = inventory->getItem(index);

    if (!item)
        return;

    ITEM_SHOP item_shop;

    sprintf(item_shop.name, "%s %i gp",
            item->getInfo()->getName().c_str(), price);
    item_shop.price = price;
    item_shop.index = index;
    item_shop.id = item->getId();;
    item_shop.quantity = item->getQuantity();

    shopInventory.push_back(item_shop);
    itemList->adjustSize();
}

void SellDialog::action(const std::string& eventId)
{
    int selectedItem = itemList->getSelected();
    std::stringstream oss;

    if (eventId == "item")
    {
        if (selectedItem > -1)
        {
            slider->setEnabled(true);
            increaseButton->setEnabled(true);
            decreaseButton->setEnabled(false);

            m_maxItems = shopInventory[selectedItem].quantity;
            m_amountItems = 0;
            slider->setValue(0);
            oss << m_amountItems;
            quantityLabel->setCaption(oss.str());
            quantityLabel->adjustSize();
        }
        else
        {
            slider->setValue(0);
            slider->setEnabled(false);
            increaseButton->setEnabled(false);
            decreaseButton->setEnabled(false);
            sellButton->setEnabled(false);
            m_amountItems = 0;
      }
    }
    else if (eventId == "slider" && selectedItem > -1)
    {
        m_amountItems = (int)(slider->getValue() * m_maxItems);

        oss << m_amountItems;
        quantityLabel->setCaption(oss.str());
        quantityLabel->adjustSize();

        if (m_amountItems > 0)
        {
            sellButton->setEnabled(true);
            decreaseButton->setEnabled(true);
        }
        else
        {
            sellButton->setEnabled(false);
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
        sellButton->setEnabled(true);
        if (m_amountItems == m_maxItems)
        {
            increaseButton->setEnabled(false);
        }

        oss << m_amountItems;
        quantityLabel->setCaption(oss.str());
        quantityLabel->adjustSize();
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
            sellButton->setEnabled(false);
        }

        oss << m_amountItems;
        quantityLabel->setCaption(oss.str());
        quantityLabel->adjustSize();
    }
    else if (eventId == "sell" && selectedItem > -1)
    {
        // Attempt sell
        assert(m_amountItems > 0 && m_amountItems <= m_maxItems);
        assert(selectedItem >= 0 && selectedItem < int(shopInventory.size()));

        WFIFOW(0) = net_w_value(0x00c9);
        WFIFOW(2) = net_w_value(8);
        WFIFOW(4) = net_w_value(shopInventory[selectedItem].index);
        WFIFOW(6) = net_w_value(m_amountItems);
        WFIFOSET(8);

        if (m_amountItems == m_maxItems)
        {
            slider->setEnabled(false);
            increaseButton->setEnabled(false);
            itemList->setSelected(-1);
            shopInventory.erase(shopInventory.begin() += selectedItem);
        }
        else
        {
            m_maxItems = shopInventory[selectedItem].quantity - m_amountItems;
        }

        decreaseButton->setEnabled(false);
        sellButton->setEnabled(false);

        m_amountItems = 0;
        slider->setValue(0);
        quantityLabel->setCaption("O");
        quantityLabel->adjustSize();
    }
    else if (eventId == "quit")
    {
        setVisible(false);
        current_npc = 0;
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
