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

#include "item_amount.h"
#include "button.h"
#include "../engine.h"
#include "../main.h"

ItemAmountWindow::ItemAmountWindow():
    Window("Select amount of items to drop.")
{
    
    // New labels
    itemAmountLabel = new gcn::Label("1");

    // New buttons
    itemAmountMinusButton = new Button("-");
    itemAmountPlusButton = new Button("+");
    itemAmountOkButton = new Button("Okay");
    itemAmountCancelButton = new Button("Cancel");
    
    // Set button events Id
    itemAmountMinusButton->setEventId("Minus");
    itemAmountPlusButton->setEventId("Plus");
    itemAmountOkButton->setEventId("Drop");
    itemAmountCancelButton->setEventId("Cancel");
    
    // Set position
    itemAmountLabel->setPosition(35, 10);
    itemAmountPlusButton->setPosition(60, 5);
    itemAmountMinusButton->setPosition(10, 5);
    itemAmountOkButton->setPosition(10, 40);
    itemAmountCancelButton->setPosition(60, 40);
    
    // Assemble
    add(itemAmountLabel);
    add(itemAmountPlusButton);
    add(itemAmountMinusButton);
    add(itemAmountOkButton);
    add(itemAmountCancelButton);

    itemAmountPlusButton->addActionListener(this);
    itemAmountMinusButton->addActionListener(this);
    itemAmountOkButton->addActionListener(this);
    itemAmountCancelButton->addActionListener(this);
   
    resetAmount();

    setContentSize(200, 80);
    setLocationRelativeTo(getParent());
}

ItemAmountWindow::~ItemAmountWindow() {
    delete itemAmountLabel;
    delete itemAmountPlusButton;
    delete itemAmountMinusButton;
    delete itemAmountOkButton;
    delete itemAmountCancelButton;
}

void ItemAmountWindow::resetAmount() {
    amount = 1;
    itemAmountLabel->setCaption("1");
}

void ItemAmountWindow::action(const std::string& eventId) {
    if (eventId == "Cancel")
    {
        resetAmount();
        setVisible(false);
    } else if (eventId == "Drop")
    {
        inventoryWindow->dropItem(inventoryWindow->items->getIndex(), amount);
        resetAmount();
        setVisible(false);
    } else if (eventId == "Plus")
    {
        if (amount < inventoryWindow->items->getQuantity())
        {
            char tmpplus[128];
            amount++;
            sprintf(tmpplus, "%i", amount);
            itemAmountLabel->setCaption(tmpplus);
            itemAmountLabel->adjustSize();
        }
    } else if (eventId == "Minus")
    {
        if (amount > 1)
        {
            char tmpminus[128];
            amount = amount - 1;
            sprintf(tmpminus, "%i", amount);
            itemAmountLabel->setCaption(tmpminus);
            itemAmountLabel->adjustSize();
        }
    }
    
    /*WFIFOW(0) = net_w_value(0x00bb);

    if (eventId == "STR") {
        WFIFOW(2) = net_w_value(0x000d);
    }
    if (eventId == "AGI") {
        WFIFOW(2) = net_w_value(0x000e);
    }
    if (eventId == "VIT") {
        WFIFOW(2) = net_w_value(0x000f);
    }
    if (eventId == "INT") {
        WFIFOW(2) = net_w_value(0x0010);
    }
    if (eventId == "DEX") {
        WFIFOW(2) = net_w_value(0x0011);
    }
    if (eventId == "LUK") {
        WFIFOW(2) = net_w_value(0x0012);
    }

    flush();
    WFIFOW(4) = net_b_value(1);
    WFIFOSET(5); */
}
