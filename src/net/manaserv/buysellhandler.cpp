/*
 *  The Mana World
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#include "net/manaserv/buysellhandler.h"

#include "beingmanager.h"
#include "item.h"
#include "localplayer.h"
#include "npc.h"

#include "gui/buy.h"
#include "gui/chat.h"
#include "gui/sell.h"

#include "net/messagein.h"

#include "net/manaserv/protocol.h"


namespace ManaServ {

BuySellHandler::BuySellHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_NPC_BUY,
        GPMSG_NPC_SELL,
        0
    };
    handledMessages = _messages;
}

void BuySellHandler::handleMessage(Net::MessageIn &msg)
{
    Being *being = beingManager->findBeing(msg.readInt16());
    if (!being || being->getType() != Being::NPC)
    {
        return;
    }

    int npcId = being->getId();

    switch (msg.getId())
    {
        case GPMSG_NPC_BUY:
        {
            BuyDialog* dialog = new BuyDialog(npcId);

            dialog->reset();
            dialog->setMoney(player_node->getMoney());

            while (msg.getUnreadLength())
            {
                int itemId = msg.readInt16();
                int amount = msg.readInt16();
                int value = msg.readInt16();
                dialog->addItem(itemId, amount, value);
            }
            break;
        }

        case GPMSG_NPC_SELL:
        {
            SellDialog* dialog = new SellDialog(npcId);

            dialog->reset();
            dialog->setMoney(player_node->getMoney());

            while (msg.getUnreadLength())
            {
                int itemId = msg.readInt16();
                int amount = msg.readInt16();
                int value = msg.readInt16();
                dialog->addItem(new Item(itemId, amount, false), value);
            }
            break;
        }
    }
}

} // namespace ManaServ
