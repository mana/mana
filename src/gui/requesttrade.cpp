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

#include <guichan/widgets/label.hpp>

#include "button.h"

#include "../localplayer.h"

RequestTradeDialog::RequestTradeDialog(const std::string &name):
    Window("Request for Trade", true)
{
    gcn::Label *nameLabel[2];
    nameLabel[0] = new gcn::Label("");
    nameLabel[1] = new gcn::Label("");
    mAcceptButton = new Button("Accept");
    mCancelButton = new Button("Cancel");

    setContentSize(260, 75);

    nameLabel[0]->setPosition(5, 30);
    nameLabel[1]->setPosition(5, 40);
    mCancelButton->setPosition(
            260 - 5 - mCancelButton->getWidth(),
            75 - 5 - mCancelButton->getHeight());
    mAcceptButton->setPosition(
            mCancelButton->getX() - 5 - mAcceptButton->getWidth(),
            mCancelButton->getY());

    mAcceptButton->setEventId("accept");
    mCancelButton->setEventId("cancel");

    mAcceptButton->addActionListener(this);
    mCancelButton->addActionListener(this);

    add(nameLabel[0]);
    add(nameLabel[1]);
    add(mAcceptButton);
    add(mCancelButton);

    nameLabel[0]->setCaption(name + " wants to trade with you.");
    nameLabel[0]->adjustSize();
    nameLabel[1]->setCaption("Do you want to accept?");
    nameLabel[1]->adjustSize();

    setLocationRelativeTo(getParent());
}

void RequestTradeDialog::action(const std::string& eventId)
{
    bool accept = false;

    if (eventId == "accept") {
        accept = true;
    }

    player_node->tradeReply(accept);
    scheduleDelete();
}
