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

#include "net/ea/buysellhandler.h"

#include "net/ea/protocol.h"

#include "net/messagein.h"

#include "beingmanager.h"
#include "inventory.h"
#include "item.h"
#include "localplayer.h"
#include "npc.h"

#include "gui/buy.h"
#include "gui/buysell.h"
#include "gui/sell.h"

#include "gui/widgets/chattab.h"

#include "utils/gettext.h"

namespace EAthena {

BuySellHandler::BuySellHandler()
{
    static const Uint16 _messages[] = {
        SMSG_NPC_BUY_SELL_CHOICE,
        SMSG_NPC_BUY,
        SMSG_NPC_SELL,
        SMSG_NPC_BUY_RESPONSE,
        SMSG_NPC_SELL_RESPONSE,
        0
    };
    handledMessages = _messages;
}

void BuySellHandler::handleMessage(MessageIn &msg)
{
    int n_items;
    switch (msg.getId())
    {
        case SMSG_NPC_BUY_SELL_CHOICE:
            buyDialog->setVisible(false);
            buyDialog->reset();
            sellDialog->setVisible(false);
            sellDialog->reset();
            current_npc = msg.readInt32();
            buySellDialog->setVisible(true);
            break;

        case SMSG_NPC_BUY:
            msg.readInt16();  // length
            n_items = (msg.getLength() - 4) / 11;
            buyDialog->reset();
            buyDialog->setMoney(player_node->getMoney());
            buyDialog->setVisible(true);

            for (int k = 0; k < n_items; k++)
            {
                int value = msg.readInt32();
                msg.readInt32();  // DCvalue
                msg.readInt8();  // type
                int itemId = msg.readInt16();
                buyDialog->addItem(itemId, 0, value);
            }
            break;

        case SMSG_NPC_SELL:
            msg.readInt16();  // length
            n_items = (msg.getLength() - 4) / 10;
            if (n_items > 0)
            {
                sellDialog->setMoney(player_node->getMoney());
                sellDialog->reset();
                sellDialog->setVisible(true);

                for (int k = 0; k < n_items; k++)
                {
                    int index = msg.readInt16() - INVENTORY_OFFSET;
                    int value = msg.readInt32();
                    msg.readInt32();  // OCvalue

                    Item *item = player_node->getInventory()->getItem(index);

                    if (item && !(item->isEquipped()))
                        sellDialog->addItem(item, value);
                }
            }
            else
            {
                localChatTab->chatLog(_("Nothing to sell."), BY_SERVER);
                current_npc = 0;
            }
            break;

        case SMSG_NPC_BUY_RESPONSE:
            if (msg.readInt8() == 0)
            {
                localChatTab->chatLog(_("Thanks for buying."), BY_SERVER);
            }
            else
            {
                // Reset player money since buy dialog already assumed purchase
                // would go fine
                buyDialog->setMoney(player_node->getMoney());
                localChatTab->chatLog(_("Unable to buy."), BY_SERVER);
            }
            break;

        case SMSG_NPC_SELL_RESPONSE:
            if (msg.readInt8() == 0)
                localChatTab->chatLog(_("Thanks for selling."), BY_SERVER);
            else
                localChatTab->chatLog(_("Unable to sell."), BY_SERVER);

            break;
    }
}

} // namespace EAthena
