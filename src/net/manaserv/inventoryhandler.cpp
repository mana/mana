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
#include "log.h"
#include "playerinfo.h"

#include "gui/inventorywindow.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/manaserv_protocol.h"

#include "resources/iteminfo.h"

extern Net::InventoryHandler *inventoryHandler;

namespace ManaServ {

struct EquipItemInfo
{

    EquipItemInfo(int itemId, int equipSlot, int amountUsed):
        mItemId(itemId), mEquipSlot(equipSlot), mAmountUsed(amountUsed)
    {}

    int mItemId, mEquipSlot, mAmountUsed;
};

extern Connection *gameServerConnection;

EquipBackend::EquipBackend()
{
    listen(Event::ClientChannel);
}

Item *EquipBackend::getEquipment(int index) const
{
    if (index < 0 || (unsigned) index >= mSlots.size())
        return 0;
    return mSlots.at(index);
}

void EquipBackend::clear()
{
    for (std::vector<Item*>::iterator i = mSlots.begin(), i_end = mSlots.end();
         i != i_end; ++i)
    {
        if (Item *item = *i)
            item->setEquipped(false);
    }
    mSlots.assign(mSlots.size(), 0);
}

void EquipBackend::equip(int itemId, int equipSlot, int amountUsed)
{
    if (equipSlot < 0 || (unsigned) equipSlot >= mSlotTypes.size())
    {
        logger->log("ManaServ::EquipBackend: Equipment slot out of range");
        return;
    }

    if (!itemDb->exists(itemId))
    {
        logger->log("ManaServ::EquipBackend: No item with id %d",
                    itemId);
        return;
    }

    const SlotType &slotType = mSlotTypes.at(equipSlot);

    Item *itemInstance = new Item(itemId, 1, true);

    // Start at first index and search upwards for free slots to place the
    // item at the given inventory slot in
    int i = slotType.firstIndex;
    const int end_i = i + slotType.count;

    for (; i < end_i && amountUsed > 0; ++i)
    {
        if (!mSlots.at(i))
        {
            mSlots[i] = itemInstance;
            --amountUsed;
        }
    }
}

void EquipBackend::unequip(int equipmentSlot)
{
    if (equipmentSlot < 0 || (unsigned) equipmentSlot >= mSlotTypes.size())
    {
        logger->log("ManaServ::EquipBackend: Equipment slot out of range");
        return;
    }

    const SlotType &slotType = mSlotTypes.at(equipmentSlot);

    // Start at first index and search upwards for free slots to place the
    // item at the given inventory slot in
    int i = slotType.firstIndex;
    const int end_i = i + slotType.count;
    bool deleteDone = false;
    for (; i < end_i; ++i)
    {
        if (mSlots.at(i))
        {
            if (!deleteDone)
            {
                delete mSlots[i];
                deleteDone = true;
            }
            mSlots[i] = 0;
        }
    }
}

void EquipBackend::event(Event::Channel, const Event &event)
{
    if (event.getType() == Event::LoadingDatabases)
        readEquipFile();
}

void EquipBackend::readEquipFile()
{
    mSlots.clear();
    mSlotTypes.clear();

    XML::Document doc("equip.xml");
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "equip-slots"))
    {
        logger->log("ManaServ::EquipBackend: Error while reading equip.xml!");
        return;
    }

    int slotCount = 0;

    for_each_xml_child_node(childNode, rootNode)
    {
        if (!xmlStrEqual(childNode->name, BAD_CAST "slot"))
            continue;

        SlotType slotType;
        slotType.name = XML::getProperty(childNode, "name", std::string());
        slotType.count = XML::getProperty(childNode, "capacity", 1);
        slotType.visible = XML::getBoolProperty(childNode, "visible", false);
        slotType.firstIndex = slotCount;

        mSlotTypes.push_back(slotType);
        slotCount += slotType.count;
    }

    mSlots.resize(slotCount);
}


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

    listen(Event::ItemChannel);
}

void InventoryHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_INVENTORY_FULL:
            {
                PlayerInfo::clearInventory();
                PlayerInfo::getEquipment()->setBackend(&mEquipBackend);
                int count = msg.readInt16();
                while (count--)
                {
                    int slot = msg.readInt16();
                    int id = msg.readInt16();
                    int amount = msg.readInt16();
                    PlayerInfo::setInventoryItem(slot, id, amount);
                }

                // A map of { item instance, {equip slot, item id, amount used}}
                std::map<int, EquipItemInfo> equipItemsInfo;
                std::map<int, EquipItemInfo>::iterator it;
                while (msg.getUnreadLength())
                {
                    int equipSlot = msg.readInt16();
                    int itemId = msg.readInt16();
                    int itemInstance = msg.readInt16();

                    // Turn the data received into a usable format
                    it = equipItemsInfo.find(itemInstance);
                    if (it == equipItemsInfo.end())
                    {
                        // Add a new entry
                        equipItemsInfo.insert(std::make_pair(itemInstance,
                            EquipItemInfo(itemId, equipSlot, 1)));
                    }
                    else
                    {
                        // Add amount to the existing entry
                        it->second.mAmountUsed++;
                    }
                }

                for (it = equipItemsInfo.begin(); it != equipItemsInfo.end();
                     ++it)
                {
                    mEquipBackend.equip(it->second.mItemId,
                                        it->second.mEquipSlot,
                                        it->second.mAmountUsed);
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
            {
                int itemId = msg.readInt16();
                int equipSlotCount = msg.readInt16();

                if (equipSlotCount <= 0)
                    break;

                // Otherwise equip the item in the given slots
                while (equipSlotCount--)
                {
                    unsigned int equipSlot = msg.readInt16();
                    unsigned int amountUsed = msg.readInt16();

                    if (amountUsed == 0)
                    {
                        // No slots means to unequip this item
                        mEquipBackend.unequip(equipSlot);
                    }
                    else
                    {
                        mEquipBackend.equip(itemId, equipSlot,
                                            amountUsed);
                    }
                }

            }
            break;
    }
}

void InventoryHandler::event(Event::Channel channel,
                             const Event &event)
{
    if (channel == Event::ItemChannel)
    {
        Item *item = event.getItem("item");

        if (!item)
            return;

        int index = item->getInvIndex();

        if (event.getType() == Event::DoEquip)
        {
            MessageOut msg(PGMSG_EQUIP);
            msg.writeInt16(index);
            gameServerConnection->send(msg);
        }
        else if (event.getType() == Event::DoUnequip)
        {
            MessageOut msg(PGMSG_UNEQUIP);
            msg.writeInt16(index);
            gameServerConnection->send(msg);
        }
        else if (event.getType() == Event::DoUse)
        {
            MessageOut msg(PGMSG_USE_ITEM);
            msg.writeInt16(index);
            gameServerConnection->send(msg);
        }
        else if (event.getType() == Event::DoDrop)
        {
            int amount = event.getInt("amount", 1);

            MessageOut msg(PGMSG_DROP);
            msg.writeInt16(index);
            msg.writeInt16(amount);
            gameServerConnection->send(msg);
        }
        else if (event.getType() == Event::DoSplit)
        {
            int amount = event.getInt("amount", 1);

            int newIndex = PlayerInfo::getInventory()->getFreeSlot();
            if (newIndex > Inventory::NO_SLOT_INDEX)
            {
                MessageOut msg(PGMSG_MOVE_ITEM);
                msg.writeInt16(index);
                msg.writeInt16(newIndex);
                msg.writeInt16(amount);
                gameServerConnection->send(msg);
            }
        }
        else if (event.getType() == Event::DoMove)
        {
            int newIndex = event.getInt("newIndex", -1);

            if (newIndex >= 0)
            {
                if (index == newIndex)
                    return;

                MessageOut msg(PGMSG_MOVE_ITEM);
                msg.writeInt16(index);
                msg.writeInt16(newIndex);
                msg.writeInt16(item->getQuantity());
                gameServerConnection->send(msg);
            }
            else
            {
                /*int source = event.getInt("source");
                int destination = event.getInt("destination");
                int amount = event.getInt("amount", 1);*/

                // TODO Support drag'n'drop to the map ground, or with other
                // windows.
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
