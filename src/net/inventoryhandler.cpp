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

#include "../resources/iteminfo.h"
#include "../item.h"
#include "../localplayer.h"

#include "../gui/chat.h"

InventoryHandler::InventoryHandler()
{
    static const Uint16 _messages[] = {
        /*
        SMSG_PLAYER_INVENTORY,
        SMSG_PLAYER_INVENTORY_ADD,
        SMSG_PLAYER_INVENTORY_REMOVE,
        SMSG_PLAYER_INVENTORY_USE,
        SMSG_ITEM_USE_RESPONSE,
        */
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
                int id = msg.readShort();
                if (slot >= 32)
                {
                    int amount = msg.readByte();
                    Item *it = player_node->getInvItem(slot - 32);
                    it->setId(id);
                    it->setQuantity(amount);
                }
            };
            break;


#if 0
        case SMSG_PLAYER_INVENTORY:
            // Only called on map load / warp. First reset all items
            // to not load them twice on map change.
            player_node->clearInventory();
            msg.readShort();  // length
            number = (msg.getLength() - 4) / 18;

            for (int loop = 0; loop < number; loop++)
            {
                index = msg.readShort();
                itemId = msg.readShort();
                msg.readByte(); // type
                msg.readByte(); // identify flag
                amount = msg.readShort();

                player_node->addInvItem(index, itemId, amount, false);

                // Trick because arrows are not considered equipment
                if (itemId == 1199 || itemId == 529)
                {
                    player_node->getInvItem(index)->setEquipment(true);
                }
            }
            break;

        case SMSG_PLAYER_INVENTORY_ADD:
            index = msg.readShort();
            amount = msg.readShort();
            itemId = msg.readShort();
            msg.readByte();  // identify flag
            msg.readByte();  // attribute
            msg.readByte();  // refine
            equipType = msg.readShort();
            msg.readByte();  // type

            if (msg.readByte()> 0) {
                chatWindow->chatLog("Unable to pick up item", BY_SERVER);
            } else {
                const ItemInfo &itemInfo = ItemDB::get(itemId);
                chatWindow->chatLog("You picked up a " +
                        itemInfo.getName(), BY_SERVER);
                player_node->addInvItem(index, itemId, amount, equipType != 0);
            }
            break;

        case SMSG_PLAYER_INVENTORY_REMOVE:
            index = msg.readShort();
            amount = msg.readShort();
            player_node->getInvItem(index)->increaseQuantity(-amount);
            break;

        case SMSG_PLAYER_INVENTORY_USE:
            index = msg.readShort();
            msg.readShort(); // item id
            msg.readLong();  // id
            amount = msg.readShort();
            msg.readByte();  // type

            player_node->getInvItem(index)->setQuantity(amount);
            break;

        case SMSG_ITEM_USE_RESPONSE:
            index = msg.readShort();
            amount = msg.readShort();

            if (msg.readByte() == 0) {
                chatWindow->chatLog("Failed to use item", BY_SERVER);
            } else {
                player_node->getInvItem(index)->setQuantity(amount);
            }
            break;
#endif

    }
}
