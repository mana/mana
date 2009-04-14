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

#include "net/tmwserv/buysellhandler.h"

#include "net/tmwserv/protocol.h"

#include "net/messagein.h"

#include "beingmanager.h"
#include "item.h"
#include "localplayer.h"
#include "npc.h"

#include "gui/buy.h"
#include "gui/chat.h"
#include "gui/sell.h"

extern BuyDialog *buyDialog;
extern SellDialog *sellDialog;
extern Window *buySellDialog;

namespace TmwServ {

BuySellHandler::BuySellHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_NPC_BUY,
        GPMSG_NPC_SELL,
        0
    };
    handledMessages = _messages;
}

void BuySellHandler::handleMessage(MessageIn &msg)
{
    Being *being = beingManager->findBeing(msg.readInt16());
    if (!being || being->getType() != Being::NPC)
    {
        return;
    }

    current_npc = being->getId();

    switch (msg.getId())
    {
        case GPMSG_NPC_BUY:
            buyDialog->reset();
            buyDialog->setMoney(player_node->getMoney());
            buyDialog->setVisible(true);

            while (msg.getUnreadLength())
            {
                int itemId = msg.readInt16();
                int amount = msg.readInt16();
                int value = msg.readInt16();
                buyDialog->addItem(itemId, amount, value);
            }
            break;

        case GPMSG_NPC_SELL:
            sellDialog->setMoney(player_node->getMoney());
            sellDialog->reset();
            sellDialog->setVisible(true);

            while (msg.getUnreadLength())
            {
                int itemId = msg.readInt16();
                int amount = msg.readInt16();
                int value = msg.readInt16();
                sellDialog->addItem(new Item(itemId, amount, false), value);
            }
            break;
    }
}

} // namespace TmwServ
