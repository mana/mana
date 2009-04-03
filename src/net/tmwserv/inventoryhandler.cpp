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

#include "net/tmwserv/inventoryhandler.h"

#include "net/tmwserv/protocol.h"

#include "net/tmwserv/gameserver/player.h"

#include "net/messagein.h"

#include "equipment.h"
#include "inventory.h"
#include "item.h"
#include "itemshortcut.h"
#include "localplayer.h"

#include "gui/chat.h"

#include "resources/iteminfo.h"

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
                int slot = msg.readInt8();
                if (slot == 255)
                {
                    player_node->setMoney(msg.readInt32());
                    continue;
                }

                int id = msg.readInt16();
                if (slot < EQUIPMENT_SIZE)
                {
                    player_node->mEquipment->setEquipment(slot, id);
                }
                else if (slot >= 32 && slot < 32 + INVENTORY_SIZE)
                {
                    int amount = id ? msg.readInt8() : 0;
                    player_node->setInvItem(slot - 32, id, amount);
                }
            };
            break;
    }
}

void InventoryHandler::equipItem(Item *item)
{
    Net::GameServer::Player::equip(item->getInvIndex());
}

void InventoryHandler::unequipItem(Item *item)
{
    Net::GameServer::Player::unequip(item->getInvIndex());
}

void InventoryHandler::useItem(Item *item)
{
    Net::GameServer::Player::useItem(item->getInvIndex());
}

void InventoryHandler::dropItem(Item *item, int amount)
{
    Net::GameServer::Player::drop(item->getInvIndex(), amount);
}

void InventoryHandler::splitItem(Item *item, int amount)
{
    // TODO
}

void InventoryHandler::openStorage()
{
    // TODO
}

void InventoryHandler::closeStorage()
{
    // TODO
}

void InventoryHandler::moveItem(StorageType source, int slot, int amount,
                      StorageType destination)
{
    // TODO
}
