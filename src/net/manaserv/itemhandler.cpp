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

#include "net/manaserv/itemhandler.h"

#include "actorspritemanager.h"

#include "net/manaserv/manaserv_protocol.h"
#include "net/manaserv/messagein.h"

#include "log.h"

namespace ManaServ {

ItemHandler::ItemHandler()
{
    static const uint16_t _messages[] = {
        GPMSG_ITEMS,
        GPMSG_ITEM_APPEAR,
        0
    };
    handledMessages = _messages;
}

void ItemHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_ITEM_APPEAR:
        case GPMSG_ITEMS:
        {
            while (msg.getUnreadLength())
            {
                int itemId = msg.readInt16();
                int x = msg.readInt16();
                int y = msg.readInt16();
                int id = (x << 16) | y; // dummy id

                if (itemId)
                {
                    actorSpriteManager->createItem(id, itemId, Vector(x, y));
                }
                else if (FloorItem *item = actorSpriteManager->findItem(id))
                {
                    actorSpriteManager->destroy(item);
                }
            }
        } break;
    }
}

} // namespace ManaServ
