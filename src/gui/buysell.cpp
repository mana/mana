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
 */

#include "button.h"
#include "buysell.h"

#include "../npc.h"

BuySellDialog::BuySellDialog():
    Window("Shop")
{
    Button *buyButton = 0;
    const char *buttonNames[] = {
        "Buy", "Sell", "Cancel", 0
    };
    int x = 10, y = 10;

    for (const char **curBtn = buttonNames; *curBtn; curBtn++)
    {
        Button *btn = new Button(*curBtn, *curBtn, this);
        if (!buyButton) buyButton = btn; // For focus request
        btn->setPosition(x, y);
        add(btn);
        x += btn->getWidth() + 10;
    }
    buyButton->requestFocus();

    setContentSize(x, 2 * y + buyButton->getHeight());
    setLocationRelativeTo(getParent());

    requestFocus();
}

void BuySellDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "Buy") {
        current_npc->buy();
    } else if (event.getId() == "Sell") {
        current_npc->sell();
    } else if (event.getId() == "Cancel") {
        current_npc = 0;
    }
    setVisible(false);
}
