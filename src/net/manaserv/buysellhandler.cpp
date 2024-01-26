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

#include "net/manaserv/buysellhandler.h"

#include "actorspritemanager.h"
#include "item.h"
#include "playerinfo.h"

#include "gui/buydialog.h"
#include "gui/selldialog.h"

#include "net/manaserv/manaserv_protocol.h"
#include "net/manaserv/messagein.h"

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

void BuySellHandler::handleMessage(MessageIn &msg)
{
    Being *being = actorSpriteManager->findBeing(msg.readInt16());
    if (!being || being->getType() != ActorSprite::NPC)
    {
        return;
    }

    int npcId = being->getId();

    switch (msg.getId())
    {
        case GPMSG_NPC_BUY:
        {
            auto* dialog = new BuyDialog(npcId);

            dialog->reset();
            dialog->setMoney(PlayerInfo::getAttribute(MONEY));

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
            auto* dialog = new SellDialog(npcId);

            dialog->reset();
            dialog->setMoney(PlayerInfo::getAttribute(MONEY));

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
