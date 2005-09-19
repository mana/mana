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

#include "requesttrade.h"

#include <sstream>
#include <guichan/widgets/label.hpp>

#include "button.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

bool requestTradeDialogOpen = false;

RequestTradeDialog::RequestTradeDialog(const std::string &name):
    Window("Request for Trade", true)
{
    nameLabel[0] = new gcn::Label("");
    nameLabel[1] = new gcn::Label("");
    acceptButton = new Button("Accept");
    cancelButton = new Button("Cancel");

    setContentSize(260, 75);

    nameLabel[0]->setPosition(5, 30);
    nameLabel[1]->setPosition(5, 40);
    cancelButton->setPosition(
            260 - 5 - cancelButton->getWidth(),
            75 - 5 - cancelButton->getHeight());
    acceptButton->setPosition(
            cancelButton->getX() - 5 - acceptButton->getWidth(),
            cancelButton->getY());

    acceptButton->setEventId("accept");
    cancelButton->setEventId("cancel");

    acceptButton->addActionListener(this);
    cancelButton->addActionListener(this);

    add(nameLabel[0]);
    add(nameLabel[1]);
    add(acceptButton);
    add(cancelButton);

    std::stringstream cap[2];
    cap[0] << name << " wants to trade with you.";
    cap[1] << "Do you want to accept?";

    nameLabel[0]->setCaption(cap[0].str());
    nameLabel[0]->adjustSize();
    nameLabel[1]->setCaption(cap[1].str());
    nameLabel[1]->adjustSize();

    setLocationRelativeTo(getParent());
}

RequestTradeDialog::~RequestTradeDialog()
{
    delete acceptButton;
    delete cancelButton;
    for (int i = 0; i < 2; i++) {
        delete nameLabel[i];
    }
}

void RequestTradeDialog::action(const std::string& eventId)
{
    int choice = 4; // 4 means trade canceled
    
    if (eventId == "accept") {
        choice = 3; // ok to trade
    }
    else if (eventId == "cancel") {
        requestTradeDialogOpen = false;
    }
    
    MessageOut outMsg;
    outMsg.writeShort(CMSG_TRADE_RESPONSE);
    outMsg.writeByte(choice);
    scheduleDelete();
}
