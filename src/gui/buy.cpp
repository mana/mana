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

#include <sstream>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "listbox.h"
#include "scrollarea.h"
#include "shop.h"
#include "slider.h"

#include "../npc.h"

#include "../resources/iteminfo.h"
#include "../resources/itemmanager.h"

#include "../net/messageout.h"
#include "../net/protocol.h"


BuyDialog::BuyDialog(Network *network):
    Window("Buy"), mNetwork(network),
    m_money(0), m_amountItems(0), m_maxItems(0)
{
    mShopItems = new ShopItems;

    itemList = new ListBox(mShopItems);
    scrollArea = new ScrollArea(itemList);
    slider = new Slider(1.0);
    quantityLabel = new gcn::Label("0");
    moneyLabel = new gcn::Label("Price: 0 GP");
    increaseButton = new Button("+");
    decreaseButton = new Button("-");
    buyButton = new Button("Buy");
    quitButton = new Button("Quit");
    itemDescLabel = new gcn::Label("Description:");
    itemEffectLabel = new gcn::Label("Effect:");

    setContentSize(260, 210);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    scrollArea->setDimension(gcn::Rectangle(5, 5, 250, 110));
    itemList->setDimension(gcn::Rectangle(5, 5, 238, 110));

    slider->setDimension(gcn::Rectangle(5, 120, 200, 10));
    slider->setEnabled(false);

    quantityLabel->setPosition(215, 120);
    moneyLabel->setPosition(5, 130);

    increaseButton->setPosition(40, 186);
    increaseButton->setSize(20, 20);
    increaseButton->setEnabled(false);

    decreaseButton->setPosition(10, 186);
    decreaseButton->setSize(20, 20);
    decreaseButton->setEnabled(false);

    buyButton->setPosition(180, 186);
    buyButton->setEnabled(false);

    quitButton->setPosition(212, 186);

    itemEffectLabel->setDimension(gcn::Rectangle(5, 150, 240, 14));
    itemDescLabel->setDimension(gcn::Rectangle(5, 169, 240, 14));

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
    add(buyButton);
    add(quitButton);
    add(increaseButton);
    add(decreaseButton);
    add(moneyLabel);
    add(itemDescLabel);
    add(itemEffectLabel);

    setLocationRelativeTo(getParent());
}

BuyDialog::~BuyDialog()
{
    delete mShopItems;
}

void BuyDialog::setMoney(int amount)
{
    m_money = amount;
}

void BuyDialog::reset()
{
    mShopItems->clear();
    m_money = 0;
    slider->setValue(0.0);
    m_amountItems = 0;

    // Reset Previous Selected Items to prevent failing asserts
    itemList->setSelected(-1);
    increaseButton->setEnabled(false);
    decreaseButton->setEnabled(false);
    quantityLabel->setCaption("0");
    quantityLabel->adjustSize();
    moneyLabel->setCaption("Price: 0");
    moneyLabel->adjustSize();
    itemDescLabel->setCaption("");
    itemEffectLabel->setCaption("");

}

void BuyDialog::addItem(short id, int price)
{
    ITEM_SHOP item_shop;
    std::stringstream ss;

    ss << itemDb->getItemInfo(id)->getName() << " " << price << " GP";
    item_shop.name = ss.str();
    item_shop.price = price;
    item_shop.id = id;

    mShopItems->push_back(item_shop);
    itemList->adjustSize();
}

void BuyDialog::action(const std::string& eventId)
{
    int selectedItem = itemList->getSelected();

    if (eventId == "item") {
        // Reset amount of items and update labels
        m_amountItems = 0;
        slider->setValue(0);
        quantityLabel->setCaption("0");
        quantityLabel->adjustSize();
        moneyLabel->setCaption("Price : 0 GP");
        moneyLabel->adjustSize();

        // Disable buttons for buying and decreasing
        buyButton->setEnabled(false);
        decreaseButton->setEnabled(false);

        // If no item was selected, none can be bought, otherwise
        // calculate how many the player can afford
        m_maxItems = (itemList->getSelected() == -1) ? 0 :
            m_money / mShopItems->at(selectedItem).price;

        // When at least one item can be bought, enable the slider and the
        // increase button
        increaseButton->setEnabled(m_maxItems > 0);
        slider->setEnabled(m_maxItems > 0);
    }
    else if (eventId == "quit") {
        setVisible(false);
        current_npc = 0;
    }

    // The following actions require a valid selection
    if (selectedItem < 0 || selectedItem >= int(mShopItems->size())) {
        return;
    }

    bool updateButtonsAndLabels = false;

    if (eventId == "slider") {
        m_amountItems = (int)(slider->getValue() * m_maxItems);
        updateButtonsAndLabels = true;
    }
    else if (eventId == "+") {
        if (m_amountItems < m_maxItems) {
            m_amountItems++;
        } else {
            m_amountItems = m_maxItems;
        }

        slider->setValue(double(m_amountItems)/double(m_maxItems));
        updateButtonsAndLabels = true;
    }
    else if (eventId == "-") {
        if (m_amountItems > 0) {
            m_amountItems--;
        } else {
            m_amountItems = 0;
        }

        slider->setValue(double(m_amountItems)/double(m_maxItems));
        updateButtonsAndLabels = true;
    }
    // TODO Actually we'd have a bug elsewhere if this check for the number
    // of items to be bought ever fails, Bertram removed the assertions, is
    // there a better way to ensure this fails in an _obivous_ way in C++?
    else if (eventId == "buy" && (m_amountItems > 0 &&
                m_amountItems <= m_maxItems)) {
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(CMSG_NPC_BUY_REQUEST);
        outMsg.writeInt16(8);
        outMsg.writeInt16(m_amountItems);
        outMsg.writeInt16(mShopItems->at(selectedItem).id);

        // update money !
        m_money -= m_amountItems * mShopItems->at(selectedItem).price;
        // Update number of items that can be bought at max
        m_maxItems -= m_amountItems;

        if (!m_maxItems) {
            slider->setEnabled(false);
        }

        // Reset selection
        m_amountItems = 0;
        slider->setValue(0);

        updateButtonsAndLabels = true;
    }

    // If anything has changed, we have to update the buttons and labels
    if (updateButtonsAndLabels) {
        std::stringstream oss;

        // Update buttons
        increaseButton->setEnabled(m_amountItems < m_maxItems);
        decreaseButton->setEnabled(m_amountItems > 0);
        buyButton->setEnabled(m_amountItems > 0);

        // Update labels
        oss << m_amountItems;
        quantityLabel->setCaption(oss.str());
        quantityLabel->adjustSize();

        oss.str("");
        oss << "Price : " << m_amountItems * mShopItems->at(selectedItem).price << " GP";
        moneyLabel->setCaption(oss.str());
        moneyLabel->adjustSize();
    }
}

void BuyDialog::mouseClick(int x, int y, int button, int count)
{
    Window::mouseClick(x, y, button, count);

    int selectedItem = itemList->getSelected();
    if (selectedItem > -1)
    {
        itemDescLabel->setCaption("Description: " +
                itemDb->getItemInfo(mShopItems->at(selectedItem).id)->getDescription());
        itemEffectLabel->setCaption("Effect: " +
                itemDb->getItemInfo(mShopItems->at(selectedItem).id)->getEffect());
    }
}
