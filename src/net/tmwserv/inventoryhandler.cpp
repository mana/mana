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

#include "net/tmwserv/connection.h"
#include "net/tmwserv/protocol.h"

#include "net/tmwserv/gameserver/internal.h"
#include "net/tmwserv/gameserver/player.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "equipment.h"
#include "inventory.h"
#include "item.h"
#include "itemshortcut.h"
#include "localplayer.h"

#include "gui/chat.h"

#include "resources/iteminfo.h"

Net::InventoryHandler *inventoryHandler;

namespace TmwServ {

InventoryHandler::InventoryHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_INVENTORY_FULL,
        GPMSG_INVENTORY,
        0
    };
    handledMessages = _messages;
    inventoryHandler = this;
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

void InventoryHandler::equipItem(const Item *item)
{
    MessageOut msg(PGMSG_EQUIP);
    msg.writeInt8(item->getInvIndex());
    Net::GameServer::connection->send(msg);
}

void InventoryHandler::unequipItem(const Item *item)
{
    MessageOut msg(PGMSG_UNEQUIP);
    msg.writeInt8(item->getInvIndex());
    Net::GameServer::connection->send(msg);

    // Tidy equipment directly to avoid weapon still shown bug, for instance
    player_node->mEquipment->setEquipment(item->getInvIndex(), 0);
}

void InventoryHandler::useItem(const Item *item)
{
    MessageOut msg(PGMSG_USE_ITEM);
    msg.writeInt8(item->getInvIndex());
    Net::GameServer::connection->send(msg);
}

void InventoryHandler::dropItem(const Item *item, int amount)
{
    MessageOut msg(PGMSG_DROP);
    msg.writeInt8(item->getInvIndex());
    msg.writeInt8(amount);
    Net::GameServer::connection->send(msg);
}

bool InventoryHandler::canSplit(const Item *item)
{
    return item && !item->isEquipment() && item->getQuantity() > 1;
}

void InventoryHandler::splitItem(const Item *item, int amount)
{
    int newIndex = player_node->getInventory()->getFreeSlot();
    if (newIndex > Inventory::NO_SLOT_INDEX)
    {
        Net::GameServer::Player::moveItem(
            item->getInvIndex(), newIndex, amount);
    }
}

void InventoryHandler::moveItem(int oldIndex, int newIndex)
{
    if (oldIndex == newIndex)
        return;

    Net::GameServer::Player::moveItem(oldIndex, newIndex,
        player_node->getInventory()->getItem(oldIndex)->getQuantity());
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

} // namespace TmwServ
