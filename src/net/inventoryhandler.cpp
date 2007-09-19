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

#include "inventoryhandler.h"

#include <SDL_types.h>

#include "messagein.h"
#include "protocol.h"

#include "../equipment.h"
#include "../inventory.h"
#include "../item.h"
#include "../itemshortcut.h"
#include "../localplayer.h"

#include "../gui/chat.h"
#include "../resources/iteminfo.h"

InventoryHandler::InventoryHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_INVENTORY_FULL,
        GPMSG_INVENTORY,
        0
    };
    handledMessages = _messages;
}

void InventoryHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_INVENTORY_FULL:
            player_node->clearInventory();
            // no break!

        case GPMSG_INVENTORY:
            while (msg.getUnreadLength())
            {
                int slot = msg.readByte();
                if (slot == 255)
                {
                    player_node->setMoney(msg.readLong());
                    continue;
                }

                int id = msg.readShort();
                if (slot < EQUIPMENT_SIZE)
                {
                    player_node->mEquipment->setEquipment(slot, id);
                }
                else if (slot >= 32 && slot < 32 + INVENTORY_SIZE)
                {
                    int amount = id ? msg.readByte() : 0;
                    Item *it = player_node->getInvItem(slot - 32);
                    it->setId(id);
                    it->setQuantity(amount);
                }
            };
            itemShortcut->load();
            break;
    }
}
