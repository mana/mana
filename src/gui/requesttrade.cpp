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
#include "button.h"
#include "../game.h"
#include "../net/network.h"

RequestTradeDialog::RequestTradeDialog(const char *name):
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
    if (eventId == "accept") {
        // Send the selected index back to the server
        WFIFOW(0) = net_w_value(0x00e6);
        WFIFOB(2) = net_b_value(3);
        WFIFOSET(3);
        scheduleDelete();
    }
    else if (eventId == "cancel") {
        // 0xff packet means cancel
        WFIFOW(0) = net_w_value(0x00e6);
        WFIFOB(2) = net_b_value(4);
        WFIFOSET(3);
        scheduleDelete();
    }
}
