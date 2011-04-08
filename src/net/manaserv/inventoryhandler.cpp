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

#include "net/manaserv/inventoryhandler.h"

#include "equipment.h"
#include "inventory.h"
#include "item.h"
#include "itemshortcut.h"
#include "localplayer.h"
#include "playerinfo.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/manaserv_protocol.h"

#include "resources/iteminfo.h"

extern Net::InventoryHandler *inventoryHandler;

namespace ManaServ {

extern Connection *gameServerConnection;

InventoryHandler::InventoryHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_INVENTORY_FULL,
        GPMSG_INVENTORY,
        GPMSG_EQUIP,
        0
    };
    handledMessages = _messages;
    inventoryHandler = this;

    listen(CHANNEL_ITEM);
}

void InventoryHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_INVENTORY_FULL:
            {
                PlayerInfo::clearInventory();
                PlayerInfo::getEquipment()->setBackend(&mEquips);
                int count = msg.readInt16();
                while (count--)
                {
                    unsigned int slot = msg.readInt16();
                    int id = msg.readInt16();
                    unsigned int amount = msg.readInt16();
                    PlayerInfo::setInventoryItem(slot, id, amount);
                }
                while (msg.getUnreadLength())
                {
                    unsigned int slot = msg.readInt8();
                    unsigned int ref = msg.readInt16();

                    mEquips.addEquipment(slot, ref);
                }
            }
            break;

        case GPMSG_INVENTORY:
            while (msg.getUnreadLength())
            {
                unsigned int slot = msg.readInt16();
                int id = msg.readInt16();
                unsigned int amount = id ? msg.readInt16() : 0;
                PlayerInfo::setInventoryItem(slot, id, amount);
            }
            break;

        case GPMSG_EQUIP:
            while (msg.getUnreadLength())
            {
                unsigned int ref = msg.readInt16();
                int count = msg.readInt8();
                while (count--)
                {
                    unsigned int slot = msg.readInt8();
                    unsigned int used = msg.readInt8();

                    mEquips.setEquipment(slot, used, ref);
                }
            }
            break;
    }
}

void InventoryHandler::event(Channels channel,
                             const Mana::Event &event)
{
    if (channel == CHANNEL_ITEM)
    {
        Item *item = event.getItem("item");

        if (!item)
            return;

        int index = item->getInvIndex();

        if (event.getType() == Mana::Event::DoEquip)
        {
            MessageOut msg(PGMSG_EQUIP);
            msg.writeInt8(index);
            gameServerConnection->send(msg);
        }
        else if (event.getType() == Mana::Event::DoUnequip)
        {
            MessageOut msg(PGMSG_UNEQUIP);
            msg.writeInt8(index);
            gameServerConnection->send(msg);

            // Tidy equipment directly to avoid weapon still shown bug,
            // for instance.
            mEquips.setEquipment(index, 0, 0);
        }
        else if (event.getType() == Mana::Event::DoUse)
        {
            MessageOut msg(PGMSG_USE_ITEM);
            msg.writeInt8(index);
            gameServerConnection->send(msg);
        }
        else if (event.getType() == Mana::Event::DoDrop)
        {
            int amount = event.getInt("amount", 1);

            MessageOut msg(PGMSG_DROP);
            msg.writeInt8(index);
            msg.writeInt8(amount);
            gameServerConnection->send(msg);
        }
        else if (event.getType() == Mana::Event::DoSplit)
        {
            int amount = event.getInt("amount", 1);

            int newIndex = PlayerInfo::getInventory()->getFreeSlot();
            if (newIndex > Inventory::NO_SLOT_INDEX)
            {
                MessageOut msg(PGMSG_MOVE_ITEM);
                msg.writeInt8(index);
                msg.writeInt8(newIndex);
                msg.writeInt8(amount);
                gameServerConnection->send(msg);
            }
        }
        else if (event.getType() == Mana::Event::DoMove)
        {
            int newIndex = event.getInt("newIndex", -1);

            if (newIndex >= 0)
            {
                if (index == newIndex)
                    return;

                MessageOut msg(PGMSG_MOVE_ITEM);
                msg.writeInt8(index);
                msg.writeInt8(newIndex);
                msg.writeInt8(item->getQuantity());
                gameServerConnection->send(msg);
            }
            else
            {
                /*int source = event.getInt("source");
                int destination = event.getInt("destination");
                int amount = event.getInt("amount", 1);*/

                // TODO
            }
        }
    }
}

bool InventoryHandler::canSplit(const Item *item)
{
    return item && !item->getInfo().getEquippable()
           && item->getQuantity() > 1;
}

size_t InventoryHandler::getSize(int type) const
{
    switch (type)
    {
        case Inventory::INVENTORY:
        case Inventory::TRADE:
            return 50;
        case Inventory::STORAGE:
            return 300;
        default:
            return 0;
    }
}

} // namespace ManaServ
