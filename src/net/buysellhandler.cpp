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
        0
    };
    handledMessages = _messages;
}

void BuySellHandler::handleMessage(MessageIn &msg)
{
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

        case SMSG_NPC_BUY:
            msg.readShort();  // length
            n_items = (msg.getLength() - 4) / 11;
            buyDialog->reset();
            buyDialog->setMoney(player_node->getMoney());
            buyDialog->setVisible(true);

            for (int k = 0; k < n_items; k++)
            {
                Sint32 value = msg.readLong();
                msg.readLong();  // DCvalue
                msg.readByte();  // type
                Sint16 itemId = msg.readShort();
                buyDialog->addItem(itemId, value);
            }
            break;

        case SMSG_NPC_SELL:
            msg.readShort();  // length
            n_items = (msg.getLength() - 4) / 10;
            if (n_items > 0) {
                sellDialog->setMoney(player_node->getMoney());
                sellDialog->reset();
                sellDialog->setVisible(true);

                for (int k = 0; k < n_items; k++)
                {
                    Sint16 index = msg.readShort();
                    Sint32 value = msg.readLong();
                    msg.readLong();  // OCvalue

                    Item *item = player_node->getInvItem(index);
                    if (item && !(item->isEquipped())) {
                        sellDialog->addItem(item, value);
                    }
                }
            }
            else {
                chatWindow->chatLog("Nothing to sell", BY_SERVER);
                current_npc = 0;
            }
            break;

        case SMSG_NPC_BUY_RESPONSE:
            if (msg.readByte() == 0) {
                chatWindow->chatLog("Thanks for buying", BY_SERVER);
            } else {
                chatWindow->chatLog("Unable to buy", BY_SERVER);
            }
            break;

        case SMSG_NPC_SELL_RESPONSE:
            if (msg.readByte() == 0) {
                chatWindow->chatLog("Thanks for selling", BY_SERVER);
            } else {
                chatWindow->chatLog("Unable to sell", BY_SERVER);
            }
            break;
#endif
    }
}
