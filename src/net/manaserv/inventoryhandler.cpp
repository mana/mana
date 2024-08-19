/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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
#include "log.h"
#include "playerinfo.h"

#include "gui/equipmentwindow.h"
#include "gui/inventorywindow.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/manaserv_protocol.h"

#include "utils/stringutils.h"

#define EQUIP_FILE "equip.xml"

extern Net::InventoryHandler *inventoryHandler;

namespace ManaServ {

extern Connection *gameServerConnection;

EquipBackend::EquipBackend()
{
    listen(Event::ClientChannel);
    mVisibleSlots = 0;
}

EquipBackend::~EquipBackend()
{
    clear();
}

Item *EquipBackend::getEquipment(int slotIndex) const
{
    auto it = mSlots.find(slotIndex);
    if (it == mSlots.end())
        return nullptr;

    return PlayerInfo::getInventory()->getItem(it->second.inventorySlot);
}

std::string EquipBackend::getSlotName(int slotIndex) const
{
    auto it = mSlots.find(slotIndex);
    return it == mSlots.end() ? std::string() : it->second.name;
}

void EquipBackend::triggerUnequip(int slotIndex) const
{
    auto item = getEquipment(slotIndex);
    if (!item)
        return;

    Event event(Event::DoUnequip);
    event.setItem("item", item);
    event.trigger(Event::ItemChannel);
}

void EquipBackend::clear()
{
    mSlots.clear();
}

void EquipBackend::equip(int inventorySlot, int equipmentSlot)
{
    auto slotIt = mSlots.find(equipmentSlot);
    if (slotIt == mSlots.end())
    {
        logger->log("ManaServ::EquipBackend: Equipment slot %i"
                    " is not existing.",
                    equipmentSlot);
        return;
    }

    slotIt->second.inventorySlot = inventorySlot;

    if (auto item = PlayerInfo::getInventory()->getItem(inventorySlot))
        item->setEquipped(true);
}

void EquipBackend::unequip(int inventorySlot)
{
    for (auto &[_, slot] : mSlots)
    {
        if (slot.inventorySlot == inventorySlot)
        {
            slot.inventorySlot = -1;

            if (auto item = PlayerInfo::getInventory()->getItem(inventorySlot))
                item->setEquipped(false);

            return;
        }
    }

    logger->log("ManaServ::EquipBackend: No equipped item found at inventory "
                "slot %i!", inventorySlot);
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
    XML::Node rootNode = doc.rootNode();

    if (!rootNode || rootNode.name() != "equip-slots")
    {
        logger->log("ManaServ::EquipBackend: Error while reading "
                    EQUIP_FILE "!");
        return;
    }

    // The current client slot index
    unsigned int slotIndex = 0;
    mVisibleSlots = 0;

    for (auto slotNode : rootNode.children())
    {
        if (slotNode.name() != "slot")
            continue;

        Slot slot;
        slot.slotTypeId = slotNode.getProperty("id", 0);
        std::string name = slotNode.getProperty("name", std::string());
        const int capacity = slotNode.getProperty("capacity", 1);
        slot.weaponSlot =  slotNode.getBoolProperty("weapon", false);
        slot.ammoSlot =  slotNode.getBoolProperty("ammo", false);

        if (slotNode.getBoolProperty("visible", false))
            ++mVisibleSlots;

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
                mSlots.insert(std::make_pair(slotIndex, std::move(slot)));
                ++slotIndex;
            }
        }

        // Read the box properties
        readBoxNode(slotNode);
    }
}

void EquipBackend::readBoxNode(XML::Node slotNode)
{
    for (auto boxNode : slotNode.children())
    {
        if (boxNode.name() != "box")
            continue;

        const int x = boxNode.getProperty("x" , 0);
        const int y = boxNode.getProperty("y" , 0);

        mBoxesPositions.emplace_back(x, y);

        std::string backgroundFile =
            boxNode.getProperty("background" , std::string());
        mBoxesBackgroundFile.push_back(backgroundFile);
    }
}

bool EquipBackend::isWeaponSlot(int slotTypeId) const
{
    for (const auto &[_, slot] : mSlots)
    {
        if (slot.slotTypeId == (unsigned)slotTypeId)
            return slot.weaponSlot;
    }
    return false;
}

bool EquipBackend::isAmmoSlot(int slotTypeId) const
{
    for (const auto &[_, slot] : mSlots)
    {
        if (slot.slotTypeId == (unsigned)slotTypeId)
            return slot.ammoSlot;
    }
    return false;
}

Position EquipBackend::getBoxPosition(unsigned int slotIndex) const
{
    if (slotIndex < mBoxesPositions.size())
        return mBoxesPositions.at(slotIndex);
    return Position(0, 0);
}

const std::string &EquipBackend::getBoxBackground(unsigned int slotIndex) const
{
    if (slotIndex < mBoxesBackgroundFile.size())
        return mBoxesBackgroundFile.at(slotIndex);
    return Net::empty;
}

InventoryHandler::InventoryHandler()
{
    static const Uint16 _messages[] = {
        GPMSG_INVENTORY_FULL,
        GPMSG_INVENTORY,
        GPMSG_EQUIP,
        GPMSG_UNEQUIP,
        0
    };
    handledMessages = _messages;
    inventoryHandler = this;

    listen(Event::ItemChannel);
}

void InventoryHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case GPMSG_INVENTORY_FULL:
            {
                PlayerInfo::clearInventory();
                int count = msg.readInt16();
                while (count--)
                {
                    const int slot = msg.readInt16();
                    const int itemId = msg.readInt16();
                    const int amount = msg.readInt16();
                    const int equipmentSlot = msg.readInt16();
                    PlayerInfo::setInventoryItem(slot, itemId, amount);

                    if (equipmentSlot > 0)
                        mEquipBackend.equip(slot, equipmentSlot);
                    else
                        mEquipBackend.unequip(slot);
                }
            }
            break;

        case GPMSG_INVENTORY:
            while (msg.getUnreadLength())
            {
                const unsigned int slot = msg.readInt16();
                const int id = msg.readInt16();
                const unsigned int amount = id ? msg.readInt16() : 0;
                PlayerInfo::setInventoryItem(slot, id, amount);
            }
            break;

        case GPMSG_EQUIP:
            {
                const int inventorySlot = msg.readInt16();
                const int equipmentSlot = msg.readInt16();
                mEquipBackend.equip(inventorySlot, equipmentSlot);
            }

        case GPMSG_UNEQUIP:
            {
                const int inventorySlot = msg.readInt16();
                mEquipBackend.unequip(inventorySlot);
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
    }
}

bool InventoryHandler::canSplit(const Item *item)
{
    return false;
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
