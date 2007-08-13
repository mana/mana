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

#include "buysellhandler.h"

#include <SDL_types.h>

#include "messagein.h"
#include "protocol.h"

#include "../beingmanager.h"
#include "../item.h"
#include "../localplayer.h"
#include "../npc.h"

#include "../gui/buy.h"
#include "../gui/chat.h"
#include "../gui/sell.h"

extern BuyDialog *buyDialog;
extern SellDialog *sellDialog;
extern Window *buySellDialog;

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
    Being *being = beingManager->findBeing(msg.readShort());
    if (!being || being->getType() != Being::NPC)
    {
        return;
    }

    current_npc = static_cast< NPC * >(being);

    switch (msg.getId())
    {
#if 0
        case SMSG_NPC_BUY_SELL_CHOICE:
            buyDialog->setVisible(false);
            buyDialog->reset();
            sellDialog->setVisible(false);
            sellDialog->reset();
            buySellDialog->setVisible(true);
            current_npc = dynamic_cast<NPC*>(beingManager->findBeing(msg.readLong()));
            break;
#endif

        case GPMSG_NPC_BUY:
            buyDialog->reset();
            buyDialog->setMoney(player_node->getMoney());
            buyDialog->setVisible(true);

            while (msg.getUnreadLength())
            {
                int itemId = msg.readShort();
                int amount = msg.readShort();
                int value = msg.readShort();
                buyDialog->addItem(itemId, amount, value);
            }
            break;

        case GPMSG_NPC_SELL:
            sellDialog->setMoney(player_node->getMoney());
            sellDialog->reset();
            sellDialog->setVisible(true);

            while (msg.getUnreadLength())
            {
                int itemId = msg.readShort();
                int amount = msg.readShort();
                int value = msg.readShort();
                sellDialog->addItem(itemId, amount, value);
            }
            break;
    }
}
