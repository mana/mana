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

#include "gui/equipmentwindow.h"
#include "gui/inventorywindow.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/manaserv_protocol.h"

#include "resources/iteminfo.h"

#include "utils/stringutils.h"

#define EQUIP_FILE "equip.xml"

extern Net::InventoryHandler *inventoryHandler;

namespace ManaServ {

struct EquipItemInfo
{

    EquipItemInfo(int itemId, int slotTypeId, int amountUsed):
        mItemId(itemId), mSlotTypeId(slotTypeId), mAmountUsed(amountUsed)
    {}

    int mItemId, mSlotTypeId, mAmountUsed;
};

extern Connection *gameServerConnection;

EquipBackend::EquipBackend()
{
    listen(Event::ClientChannel);
}

Item *EquipBackend::getEquipment(int slotIndex) const
{
    Slots::const_iterator it = mSlots.find(slotIndex);
    return it == mSlots.end() ? 0 : it->second.item;
}

std::string EquipBackend::getSlotName(int slotIndex) const
{
    Slots::const_iterator it = mSlots.find(slotIndex);
    return it == mSlots.end() ? std::string() : it->second.name;
}

void EquipBackend::triggerUnequip(int slotIndex) const
{
    // First get the itemInstance
    Slots::const_iterator it = mSlots.find(slotIndex);

    if (it == mSlots.end() || it->second.itemInstance == 0 || !it->second.item)
        return;

    Event event(Event::DoUnequip);
    event.setItem("item", it->second.item);
    event.setInt("itemInstance", it->second.itemInstance);
    event.trigger(Event::ItemChannel);
}


void EquipBackend::clear()
{
    for (Slots::iterator i = mSlots.begin(), i_end = mSlots.end();
        i != i_end; ++i)
    {
        if (i->second.item)
        {
            delete i->second.item;
            i->second.item = 0;
        }
    }
    mSlots.clear();
}

void EquipBackend::equip(int itemId, int slotTypeId, int amountUsed,
                         int itemInstance)
{
    if (itemInstance <= 0)
    {
        logger->log("ManaServ::EquipBackend: Equipment slot %i"
                    " has an invalid item instance.", slotTypeId);
        return;
    }

    Slots::iterator it = mSlots.begin();
    Slots::iterator it_end = mSlots.end();
    bool slotTypeFound = false;
    for (; it != it_end; ++it)
        if (it->second.slotTypeId == (unsigned)slotTypeId)
            slotTypeFound = true;

    if (!slotTypeFound)
    {
        logger->log("ManaServ::EquipBackend: Equipment slot %i"
                    " is not existing.", slotTypeId);
        return;
    }

    if (!itemDb->exists(itemId))
    {
        logger->log("ManaServ::EquipBackend: No item with id %d",
                    itemId);
        return;
    }

    // Place the item in the slots with corresponding id until
    // the capacity requested has been reached
    for (it = mSlots.begin(); it != it_end && amountUsed > 0; ++it)
    {
        // If we're on the right slot type and that its unit
        // isn't already equipped, we can equip there.
        // The slots are already sorted by id, and subId anyway.
        if (it->second.slotTypeId == (unsigned)slotTypeId
            && (!it->second.itemInstance) && (!it->second.item))
        {
            it->second.itemInstance = itemInstance;
            it->second.item = new Item(itemId, 1, true);
            --amountUsed;
        }
    }
}

void EquipBackend::unequip(int itemInstance)
{
    Slots::iterator it = mSlots.begin();
    Slots::iterator it_end = mSlots.end();
    bool itemInstanceFound = false;
    for (; it != it_end; ++it)
        if (it->second.itemInstance == (unsigned)itemInstance)
            itemInstanceFound = true;

    if (!itemInstanceFound)
    {
        logger->log("ManaServ::EquipBackend: Equipment item instance %i"
                    " is not existing. The item couldn't be unequipped!",
                    itemInstance);
        return;
    }

    for (it = mSlots.begin(); it != it_end; ++it)
    {
        if (it->second.itemInstance != (unsigned)itemInstance)
        continue;

        // We remove the item
        it->second.itemInstance = 0;
        // We also delete the item objects
        if (it->second.item)
        {
            delete it->second.item;
            it->second.item = 0;
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
    clear();

    XML::Document doc(EQUIP_FILE);
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "equip-slots"))
    {
        logger->log("ManaServ::EquipBackend: Error while reading "
                    EQUIP_FILE "!");
        return;
    }

    // The current client slot index
    unsigned int slotIndex = 0;

    for_each_xml_child_node(childNode, rootNode)
    {
        if (!xmlStrEqual(childNode->name, BAD_CAST "slot"))
            continue;

        Slot slot;
        slot.slotTypeId = XML::getProperty(childNode, "id", 0);
        std::string name = XML::getProperty(childNode, "name", std::string());
        const int capacity = XML::getProperty(childNode, "capacity", 1);

        if (slot.slotTypeId > 0 && capacity > 0)
        {
            if (name.empty())
                slot.name = toString(slot.slotTypeId);
            else
                slot.name = name;

            // The map is filled until the capacity is reached
            for (int i = 1; i < capacity + 1; ++i)
            {
                // Add the capacity part in the name
                // when there is more than one slot unit. i.e: 1/3, 2/3
                if (capacity > 1)
                {
                    slot.name = name + " " + toString(i)
                        + "/" + toString(capacity);
                }

                slot.subId = i;
                mSlots.insert(std::make_pair(slotIndex, slot));
                ++slotIndex;
            }
        }
    }
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

                // A map of { item instance, {slot type id, item id, amount used}}
                std::map<int, EquipItemInfo> equipItemsInfo;
                std::map<int, EquipItemInfo>::iterator it;
                while (msg.getUnreadLength())
                {
                    int slotTypeId = msg.readInt16();
                    int itemId = msg.readInt16();
                    int itemInstance = msg.readInt16();

                    // Turn the data received into a usable format
                    it = equipItemsInfo.find(itemInstance);
                    if (it == equipItemsInfo.end())
                    {
                        // Add a new entry
                        equipItemsInfo.insert(std::make_pair(itemInstance,
                            EquipItemInfo(itemId, slotTypeId, 1)));
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
                                        it->second.mSlotTypeId,
                                        it->second.mAmountUsed,
                                        it->first);
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
                    unsigned int parameter = msg.readInt16();
                    unsigned int amountUsed = msg.readInt16();

                    if (amountUsed == 0)
                    {
                        // No amount means to unequip this item
                        // Note that in that case, the parameter is
                        // in fact the itemInstanceId
                        mEquipBackend.unequip(parameter);
                    }
                    else
                    {
                        int itemInstance = msg.readInt16();
                        // The parameter is in that case the slot type id.
                        mEquipBackend.equip(itemId, parameter,
                                            amountUsed, itemInstance);
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
        int itemInstance = event.getInt("itemInstance", 0);

        if (!item && itemInstance == 0)
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
            msg.writeInt16(itemInstance);
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
    return item && item->getQuantity() > 1;
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
