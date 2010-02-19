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

#include "net/manaserv/itemhandler.h"

#include "flooritemmanager.h"

#include "net/manaserv/protocol.h"
#include "net/manaserv/messagein.h"

#include "game.h"
#include "map.h"
#include "log.h"

namespace ManaServ {

ItemHandler::ItemHandler()
{
    static const Uint16 _messages[] = {
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
                    if (Game *game = Game::instance())
                    {
                        if (Map *map = game->getCurrentMap())
                        {
                            floorItemManager->create(id,
                                                     itemId,
                                                     x / map->getTileWidth(),
                                                     y / map->getTileHeight());
                        }
                        else
                        {
                            logger->log(
                                    "ItemHandler: An item wasn't created "
                                    "because of Game/Map not initialized...");
                        }
                    }
                }
                else if (FloorItem *item = floorItemManager->findById(id))
                {
                    floorItemManager->destroy(item);
                }
            }
        } break;
    }
}

} // namespace ManaServ
