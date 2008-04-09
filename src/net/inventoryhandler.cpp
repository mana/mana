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
#include "../itemshortcut.h"
#include "../localplayer.h"

#include "../gui/chat.h"

InventoryHandler::InventoryHandler()
{
    static const Uint16 _messages[] = {
        SMSG_PLAYER_INVENTORY,
        SMSG_PLAYER_INVENTORY_ADD,
        SMSG_PLAYER_INVENTORY_REMOVE,
        SMSG_PLAYER_INVENTORY_USE,
        SMSG_ITEM_USE_RESPONSE,
        0
    };
    handledMessages = _messages;
}

void InventoryHandler::handleMessage(MessageIn *msg)
{
    Sint32 number;
    Sint16 index, amount, itemId, equipType;

    switch (msg->getId())
    {
        case SMSG_PLAYER_INVENTORY:
            // Only called on map load / warp. First reset all items
            // to not load them twice on map change.
            player_node->clearInventory();
            msg->readInt16();  // length
            number = (msg->getLength() - 4) / 18;

            for (int loop = 0; loop < number; loop++)
            {
                index = msg->readInt16();
                itemId = msg->readInt16();
                msg->readInt8(); // type
                msg->readInt8(); // identify flag
                amount = msg->readInt16();
                msg->skip(2);    // unknown
                msg->skip(8);    // card (4 shorts)

                player_node->addInvItem(index, itemId, amount, false);

                // Trick because arrows are not considered equipment
                if (itemId == 1199 || itemId == 529)
                {
                    player_node->getInvItem(index)->setEquipment(true);
                }
            }
            itemShortcut->load();
            break;

        case SMSG_PLAYER_INVENTORY_ADD:
            index = msg->readInt16();
            amount = msg->readInt16();
            itemId = msg->readInt16();
            msg->readInt8();  // identify flag
            msg->readInt8();  // attribute
            msg->readInt8();  // refine
            msg->skip(8);     // card
            equipType = msg->readInt16();
            msg->readInt8();  // type

            if (msg->readInt8()> 0) {
                chatWindow->chatLog("Unable to pick up item", BY_SERVER);
            } else {
                const ItemInfo &itemInfo = ItemDB::get(itemId);
                chatWindow->chatLog("You picked up a " +
                        itemInfo.getName(), BY_SERVER);
                player_node->addInvItem(index, itemId, amount, equipType != 0);
            }
            break;

        case SMSG_PLAYER_INVENTORY_REMOVE:
            index = msg->readInt16();
            amount = msg->readInt16();
            player_node->getInvItem(index)->increaseQuantity(-amount);
            break;

        case SMSG_PLAYER_INVENTORY_USE:
            index = msg->readInt16();
            msg->readInt16(); // item id
            msg->readInt32();  // id
            amount = msg->readInt16();
            msg->readInt8();  // type

            player_node->getInvItem(index)->setQuantity(amount);
            break;

        case SMSG_ITEM_USE_RESPONSE:
            index = msg->readInt16();
            amount = msg->readInt16();

            if (msg->readInt8() == 0) {
                chatWindow->chatLog("Failed to use item", BY_SERVER);
            } else {
                player_node->getInvItem(index)->setQuantity(amount);
            }
            break;
    }
}
