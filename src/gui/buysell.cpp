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

#include "buysell.h"

#include "button.h"

#include "../game.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

BuySellDialog::BuySellDialog():
    Window("Shop")
{
    buyButton = new Button("Buy");
    sellButton = new Button("Sell");
    cancelButton = new Button("Cancel");

    buyButton->setPosition(10, 10);
    sellButton->setPosition(
            buyButton->getX() + buyButton->getWidth() + 10, 10);
    cancelButton->setPosition(
            sellButton->getX() + sellButton->getWidth() + 10, 10);
    setContentSize(cancelButton->getX() + cancelButton->getWidth() + 10,
            cancelButton->getY() + cancelButton->getHeight() + 10);

    buyButton->setEventId("buy");
    sellButton->setEventId("sell");
    cancelButton->setEventId("cancel");

    buyButton->addActionListener(this);
    sellButton->addActionListener(this);
    cancelButton->addActionListener(this);

    add(buyButton);
    add(sellButton);
    add(cancelButton);

    setLocationRelativeTo(getParent());
    buyButton->requestFocus();
}

BuySellDialog::~BuySellDialog()
{
    delete buyButton;
    delete sellButton;
    delete cancelButton;
}

void BuySellDialog::action(const std::string& eventId)
{
    int actionId = -1;

    if (eventId == "buy") {
        actionId = 0;
    }
    else if (eventId == "sell") {
        actionId = 1;
    } else if (eventId == "cancel") {
        current_npc = 0;
    }
    if (actionId > -1) {
        MessageOut outMsg;
        outMsg.writeInt16(CMSG_NPC_BUY_SELL_REQUEST);
        outMsg.writeInt32(current_npc);
        outMsg.writeInt8(actionId);
    }

    setVisible(false);
}
