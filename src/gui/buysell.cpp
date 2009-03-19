/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "button.h"
#include "buysell.h"

#include "../npc.h"

#include "../net/messageout.h"
#include "../net/protocol.h"

#include "../utils/gettext.h"

BuySellDialog::BuySellDialog(Network *network):
    Window(_("Shop")), mNetwork(network)
{
    setWindowName("BuySell");
    Button *buyButton = 0;
    static const char *buttonNames[] = {
        N_("Buy"), N_("Sell"), N_("Cancel"), 0
    };
    int x = 10, y = 10;

    for (const char **curBtn = buttonNames; *curBtn; curBtn++)
    {
        Button *btn = new Button(gettext(*curBtn), *curBtn, this);
        if (!buyButton) buyButton = btn; // For focus request
        btn->setPosition(x, y);
        add(btn);
        x += btn->getWidth() + 10;
    }
    buyButton->requestFocus();

    setContentSize(x, 2 * y + buyButton->getHeight());

    center();
    setDefaultSize();
    loadWindowState();
}

void BuySellDialog::logic()
{
    Window::logic();

    if (isVisible() && !current_npc) {
        setVisible(false);
    }
}

void BuySellDialog::setVisible(bool visible)
{
    Window::setVisible(visible);

    if (visible) requestFocus();
}

void BuySellDialog::action(const gcn::ActionEvent &event)
{
    setVisible(false);
    int action = 0;

    NPC::isTalking = false;

    if (event.getId() == "Buy")
    {
        action = 0;
    }
    else if (event.getId() == "Sell")
    {
        action = 1;
    }
    else if (event.getId() == "Cancel")
    {
        current_npc = 0;
        return;
    }

    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(current_npc);
    outMsg.writeInt8(action);
}
