/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "net/tmwa/buysellhandler.h"

#include "actorspritemanager.h"
#include "inventory.h"
#include "item.h"
#include "localplayer.h"
#include "playerinfo.h"

#include "gui/buy.h"
#include "gui/buysell.h"
#include "gui/sell.h"

#include "gui/widgets/chattab.h"

#include "net/messagein.h"

#include "net/tmwa/protocol.h"

#include "utils/gettext.h"

namespace TmwAthena {

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
    mNpcId = 0;
    handledMessages = _messages;
}

void BuySellHandler::handleMessage(Net::MessageIn &msg)
{
    int n_items;

    switch (msg.getId())
    {
        case SMSG_NPC_BUY_SELL_CHOICE:
            if (!BuySellDialog::isActive())
            {
                mNpcId = msg.readInt32();
                new BuySellDialog(mNpcId);
            }
            break;

        case SMSG_NPC_BUY:
            msg.readInt16();  // length
            n_items = (msg.getLength() - 4) / 11;
            mBuyDialog = new BuyDialog(mNpcId);
            mBuyDialog->setMoney(PlayerInfo::getAttribute(MONEY));

            for (int k = 0; k < n_items; k++)
            {
                int value = msg.readInt32();
                msg.readInt32();  // DCvalue
                msg.readInt8();  // type
                int itemId = msg.readInt16();
                mBuyDialog->addItem(itemId, 0, value);
            }
            break;

        case SMSG_NPC_SELL:
            msg.readInt16();  // length
            n_items = (msg.getLength() - 4) / 10;
            if (n_items > 0)
            {
                SellDialog *dialog = new SellDialog(mNpcId);
                dialog->setMoney(PlayerInfo::getAttribute(MONEY));

                for (int k = 0; k < n_items; k++)
                {
                    int index = msg.readInt16() - INVENTORY_OFFSET;
                    int value = msg.readInt32();
                    msg.readInt32();  // OCvalue

                    Item *item = player_node->getInventory()->getItem(index);

                    if (item && !(item->isEquipped()))
                        dialog->addItem(item, value);
                }
            }
            else
            {
                localChatTab->chatLog(_("Nothing to sell."), BY_SERVER);
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
                mBuyDialog->setMoney(PlayerInfo::getAttribute(MONEY));
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

} // namespace TmwAthena
