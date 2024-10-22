/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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
#include "event.h"
#include "inventory.h"
#include "item.h"
#include "localplayer.h"
#include "playerinfo.h"

#include "gui/buydialog.h"
#include "gui/buyselldialog.h"
#include "gui/selldialog.h"

#include "net/tmwa/messagein.h"
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

void BuySellHandler::handleMessage(MessageIn &msg)
{
    int n_items;

    switch (msg.getId())
    {
        case SMSG_NPC_BUY_SELL_CHOICE:
            if (PlayerInfo::getBuySellState() != BUYSELL_CHOOSING)
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
                auto *dialog = new SellDialog(mNpcId);
                dialog->setMoney(PlayerInfo::getAttribute(MONEY));

                for (int k = 0; k < n_items; k++)
                {
                    int index = msg.readInt16() - INVENTORY_OFFSET;
                    int value = msg.readInt32();
                    msg.readInt32();  // OCvalue

                    Item *item = PlayerInfo::getInventory()->getItem(index);

                    if (item && !(item->isEquipped()))
                        dialog->addItem(item, value);
                }
            }
            else
            {
                serverNotice(_("Nothing to sell."));
            }
            break;

        case SMSG_NPC_BUY_RESPONSE:
            if (msg.readInt8() != 0)
            {
                // Reset player money since buy dialog already assumed purchase
                // would go fine
                mBuyDialog->setMoney(PlayerInfo::getAttribute(MONEY));
                serverNotice(_("Unable to buy."));
            }
            break;

        case SMSG_NPC_SELL_RESPONSE:
            if (msg.readInt8() != 0)
                serverNotice(_("Unable to sell."));
            break;
    }
}

} // namespace TmwAthena
