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

BuySellDialog::BuySellDialog(gcn::Container *parent, gcn::ActionListener *al):
    Window(parent, "Shop")
{
    buyButton = new Button("Buy");
    sellButton = new Button("Sell");
    cancelButton = new Button("Cancel");

    buyButton->setPosition(10, 10);
    sellButton->setPosition(
            buyButton->getX() + buyButton->getWidth() + 10, 10);
    cancelButton->setPosition(
            sellButton->getX() + sellButton->getWidth() + 10, 10);
    setSize(cancelButton->getX() + cancelButton->getWidth() + 10,
            cancelButton->getY() + cancelButton->getHeight() + 10);

    buyButton->setEventId("buy");
    sellButton->setEventId("sell");
    cancelButton->setEventId("cancel");

    buyButton->addActionListener(al);
    sellButton->addActionListener(al);
    cancelButton->addActionListener(al);

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
    if (eventId == "cancel") {
    }
}
