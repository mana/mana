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

#include "net/tmwa/inventoryhandler.h"

#include "configuration.h"
#include "equipment.h"
#include "inventory.h"
#include "item.h"
#include "itemshortcut.h"
#include "localplayer.h"
#include "log.h"

#include "gui/widgets/chattab.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "net/tmwa/protocol.h"

#include "resources/iteminfo.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <SDL_types.h>

extern Net::InventoryHandler *inventoryHandler;

const Equipment::Slot EQUIP_POINTS[Equipment::EQUIP_VECTOREND] = {
    Equipment::EQUIP_LEGS_SLOT,
    Equipment::EQUIP_FIGHT1_SLOT,
    Equipment::EQUIP_GLOVES_SLOT,
    Equipment::EQUIP_RING2_SLOT,
    Equipment::EQUIP_RING1_SLOT,
    Equipment::EQUIP_FIGHT2_SLOT,
    Equipment::EQUIP_FEET_SLOT,
    Equipment::EQUIP_NECK_SLOT,
    Equipment::EQUIP_HEAD_SLOT,
    Equipment::EQUIP_TORSO_SLOT,
    Equipment::EQUIP_PROJECTILE_SLOT};

namespace TmwAthena {

int getSlot(int eAthenaSlot)
{
    if (eAthenaSlot == 0)
    {
        return Equipment::EQUIP_VECTOREND;
    }

    if (eAthenaSlot & 0x8000)
        return Equipment::EQUIP_PROJECTILE_SLOT;

    int mask = 1;
    int position = 0;
    while (!(eAthenaSlot & mask))
    {
        mask <<= 1;
        position++;
    }
    return EQUIP_POINTS[position];
}

enum { debugInventory = 1 };

InventoryHandler::InventoryHandler()
{
    static const Uint16 _messages[] = {
        SMSG_PLAYER_INVENTORY,
        SMSG_PLAYER_INVENTORY_ADD,
        SMSG_PLAYER_INVENTORY_REMOVE,
        SMSG_PLAYER_INVENTORY_USE,
        SMSG_ITEM_USE_RESPONSE,
        SMSG_PLAYER_STORAGE_ITEMS,
        SMSG_PLAYER_STORAGE_EQUIP,
        SMSG_PLAYER_STORAGE_STATUS,
        SMSG_PLAYER_STORAGE_ADD,
        SMSG_PLAYER_STORAGE_REMOVE,
        SMSG_PLAYER_STORAGE_CLOSE,
        SMSG_PLAYER_EQUIPMENT,
        SMSG_PLAYER_EQUIP,
        SMSG_PLAYER_UNEQUIP,
        SMSG_PLAYER_ARROW_EQUIP,
        SMSG_PLAYER_ATTACK_RANGE,
        0
    };
    handledMessages = _messages;
    inventoryHandler = this;

    mStorage = 0;
    mStorageWindow = 0;
}

InventoryHandler::~InventoryHandler()
{
    if (mStorageWindow)
    {
        mStorageWindow->close();
        mStorageWindow = 0;
    }

    delete mStorage;
}

void InventoryHandler::handleMessage(Net::MessageIn &msg)
{
    int number, flag;
    int index, amount, itemId, equipType, arrow;
    int identified, cards[4], itemType;
    Inventory *inventory = player_node->getInventory();

    switch (msg.getId())
    {
        case SMSG_PLAYER_INVENTORY:
        case SMSG_PLAYER_STORAGE_ITEMS:
            if (msg.getId() == SMSG_PLAYER_INVENTORY)
            {
                // Clear inventory - this will be a complete refresh
                mEquips.clear();
                player_node->mEquipment->setBackend(&mEquips);

                inventory->clear();
            }
            else
            {
                mInventoryItems.clear();
            }

            msg.readInt16();  // length
            number = (msg.getLength() - 4) / 18;

            for (int loop = 0; loop < number; loop++)
            {
                index = msg.readInt16();
                itemId = msg.readInt16();
                itemType = msg.readInt8();
                identified = msg.readInt8();
                amount = msg.readInt16();
                arrow = msg.readInt16();
                for (int i = 0; i < 4; i++)
                    cards[i] = msg.readInt16();

                index -= (msg.getId() == SMSG_PLAYER_INVENTORY) ?
                         INVENTORY_OFFSET : STORAGE_OFFSET;

                if (debugInventory)
                {
                    logger->log("Index: %d, ID: %d, Type: %d, Identified: %d, "
                                "Qty: %d, Cards: %d, %d, %d, %d",
                                index, itemId, itemType, identified, amount,
                                cards[0], cards[1], cards[2], cards[3]);
                }

                if (msg.getId() == SMSG_PLAYER_INVENTORY)
                {
                    // Trick because arrows are not considered equipment
                    bool isEquipment = arrow & 0x8000;

                    inventory->setItem(index, itemId, amount, isEquipment);
                }
                else
                {
                    mInventoryItems.push_back(InventoryItem(index, itemId,
                                                            amount, false));
                }
            }
            break;

        case SMSG_PLAYER_STORAGE_EQUIP:
            msg.readInt16();  // length
            number = (msg.getLength() - 4) / 20;

            for (int loop = 0; loop < number; loop++)
            {
                index = msg.readInt16() - STORAGE_OFFSET;
                itemId = msg.readInt16();
                itemType = msg.readInt8();
                identified = msg.readInt8();
                amount = 1;
                msg.readInt16();    // Equip Point?
                msg.readInt16();    // Another Equip Point?
                msg.readInt8();   // Attribute (broken)
                msg.readInt8();   // Refine level
                for (int i = 0; i < 4; i++)
                    cards[i] = msg.readInt16();

                if (debugInventory)
                {
                    logger->log("Index: %d, ID: %d, Type: %d, Identified: %d, "
                                "Qty: %d, Cards: %d, %d, %d, %d",
                                index, itemId, itemType, identified, amount,
                                cards[0], cards[1], cards[2], cards[3]);
                }

                mInventoryItems.push_back(InventoryItem(index, itemId, amount,
                                                        false));
            }
            break;

        case SMSG_PLAYER_INVENTORY_ADD:
            index = msg.readInt16() - INVENTORY_OFFSET;
            amount = msg.readInt16();
            itemId = msg.readInt16();
            identified = msg.readInt8();
            msg.readInt8();  // attribute
            msg.readInt8();  // refine
            for (int i = 0; i < 4; i++)
                cards[i] = msg.readInt16();
            equipType = msg.readInt16();
            itemType = msg.readInt8();

            {
                const ItemInfo &itemInfo = ItemDB::get(itemId);

                if (msg.readInt8() > 0)
                {
                    player_node->pickedUp(itemInfo, 0);
                }
                else
                {
                    player_node->pickedUp(itemInfo, amount);

                    Item *item = inventory->getItem(index);

                    if  (item && item->getId() == itemId)
                        amount += inventory->getItem(index)->getQuantity();

                    inventory->setItem(index, itemId, amount, equipType != 0);
                }
            } break;

        case SMSG_PLAYER_INVENTORY_REMOVE:
            index = msg.readInt16() - INVENTORY_OFFSET;
            amount = msg.readInt16();
            if (Item *item = inventory->getItem(index))
            {
                item->increaseQuantity(-amount);
                if (item->getQuantity() == 0)
                    inventory->removeItemAt(index);
            }
            break;

        case SMSG_PLAYER_INVENTORY_USE:
            index = msg.readInt16() - INVENTORY_OFFSET;
            msg.readInt16(); // item id
            msg.readInt32();  // id
            amount = msg.readInt16();
            msg.readInt8();  // type

            if (Item *item = inventory->getItem(index))
                item->setQuantity(amount);
            break;

        case SMSG_ITEM_USE_RESPONSE:
            index = msg.readInt16() - INVENTORY_OFFSET;
            amount = msg.readInt16();

            if (msg.readInt8() == 0)
            {
                localChatTab->chatLog(_("Failed to use item."), BY_SERVER);
            }
            else
            {
                if (Item *item = inventory->getItem(index))
                    item->setQuantity(amount);
            }
            break;

        case SMSG_PLAYER_STORAGE_STATUS:
            /*
             * This is the closest we get to an "Open Storage" packet from the
             * server. It always comes after the two SMSG_PLAYER_STORAGE_...
             * packets that update storage contents.
             */
            {
                msg.readInt16(); // Used count
                int size = msg.readInt16(); // Max size

                if (!mStorage)
                    mStorage = new Inventory(Inventory::STORAGE, size);

                InventoryItems::iterator it = mInventoryItems.begin();
                InventoryItems::iterator it_end = mInventoryItems.end();
                for (; it != it_end; it++)
                    mStorage->setItem((*it).slot, (*it).id, (*it).quantity,
                                      (*it).equip);
                mInventoryItems.clear();

                if (!mStorageWindow)
                    mStorageWindow = new InventoryWindow(mStorage);
            }
            break;

        case SMSG_PLAYER_STORAGE_ADD:
            // Move an item into storage
            index = msg.readInt16() - STORAGE_OFFSET;
            amount = msg.readInt32();
            itemId = msg.readInt16();
            identified = msg.readInt8();
            msg.readInt8();  // attribute
            msg.readInt8();  // refine
            for (int i = 0; i < 4; i++)
                cards[i] = msg.readInt16();

            if (Item *item = mStorage->getItem(index))
            {
                item->setId(itemId);
                item->increaseQuantity(amount);
            }
            else
            {
                mStorage->setItem(index, itemId, amount, false);
            }
            break;

        case SMSG_PLAYER_STORAGE_REMOVE:
            // Move an item out of storage
            index = msg.readInt16() - STORAGE_OFFSET;
            amount = msg.readInt16();
            if (Item *item = mStorage->getItem(index))
            {
                item->increaseQuantity(-amount);
                if (item->getQuantity() == 0)
                    mStorage->removeItemAt(index);
            }
            break;

        case SMSG_PLAYER_STORAGE_CLOSE:
            // Storage access has been closed

            // Storage window deletes itself
            mStorageWindow = 0;

            mStorage->clear();
            delete mStorage;
            mStorage = 0;
            break;

        case SMSG_PLAYER_EQUIPMENT:
            msg.readInt16(); // length
            number = (msg.getLength() - 4) / 20;

            for (int loop = 0; loop < number; loop++)
            {
                index = msg.readInt16() - INVENTORY_OFFSET;
                itemId = msg.readInt16();
                msg.readInt8();  // type
                msg.readInt8();  // identify flag
                msg.readInt16(); // equip type
                equipType = msg.readInt16();
                msg.readInt8();  // attribute
                msg.readInt8();  // refine
                msg.skip(8);     // card

                inventory->setItem(index, itemId, 1, true);

                if (equipType)
                {
                    mEquips.setEquipment(getSlot(equipType), index);
                }
            }
            break;

        case SMSG_PLAYER_EQUIP:
            index = msg.readInt16() - INVENTORY_OFFSET;
            equipType = msg.readInt16();
            flag = msg.readInt8();

            if (!flag)
                localChatTab->chatLog(_("Unable to equip."), BY_SERVER);
            else
                mEquips.setEquipment(getSlot(equipType), index);
            break;

        case SMSG_PLAYER_UNEQUIP:
            index = msg.readInt16() - INVENTORY_OFFSET;
            equipType = msg.readInt16();
            flag = msg.readInt8();

            if (!flag)
                localChatTab->chatLog(_("Unable to unequip."), BY_SERVER);
            else
                mEquips.setEquipment(getSlot(equipType), -1);
            break;

        case SMSG_PLAYER_ATTACK_RANGE:
            player_node->setAttackRange(msg.readInt16());
            break;

        case SMSG_PLAYER_ARROW_EQUIP:
            index = msg.readInt16();

            if (index <= 1)
                break;

            index -= INVENTORY_OFFSET;

            logger->log("Arrows equipped: %i", index);
            mEquips.setEquipment(Equipment::EQUIP_PROJECTILE_SLOT, index);
            break;
    }
}

void InventoryHandler::equipItem(const Item *item)
{
    if (!item)
        return;

    MessageOut outMsg(CMSG_PLAYER_EQUIP);
    outMsg.writeInt16(item->getInvIndex() + INVENTORY_OFFSET);
    outMsg.writeInt16(0);
}

void InventoryHandler::unequipItem(const Item *item)
{
    if (!item)
        return;

    MessageOut outMsg(CMSG_PLAYER_UNEQUIP);
    outMsg.writeInt16(item->getInvIndex() + INVENTORY_OFFSET);
}

void InventoryHandler::useItem(const Item *item)
{
    if (!item)
        return;

    MessageOut outMsg(CMSG_PLAYER_INVENTORY_USE);
    outMsg.writeInt16(item->getInvIndex() + INVENTORY_OFFSET);
    outMsg.writeInt32(item->getId()); // unused
}

void InventoryHandler::dropItem(const Item *item, int amount)
{
    // TODO: Fix wrong coordinates of drops, serverside? (what's wrong here?)
    MessageOut outMsg(CMSG_PLAYER_INVENTORY_DROP);
    outMsg.writeInt16(item->getInvIndex() + INVENTORY_OFFSET);
    outMsg.writeInt16(amount);
}

bool InventoryHandler::canSplit(const Item *item)
{
    return false;
}

void InventoryHandler::splitItem(const Item *item, int amount)
{
    // Not implemented for eAthena (possible?)
}

void InventoryHandler::moveItem(int oldIndex, int newIndex)
{
    // Not implemented for eAthena (possible?)
}

void InventoryHandler::openStorage(int type)
{
    // Doesn't apply to eAthena, since opening happens through NPCs?
}

void InventoryHandler::closeStorage(int type)
{
    MessageOut outMsg(CMSG_CLOSE_STORAGE);
}

void InventoryHandler::moveItem(int source, int slot, int amount,
                                int destination)
{
    if (source == Inventory::INVENTORY && destination == Inventory::STORAGE)
    {
        MessageOut outMsg(CMSG_MOVE_TO_STORAGE);
        outMsg.writeInt16(slot + INVENTORY_OFFSET);
        outMsg.writeInt32(amount);
    }
    else if (source == Inventory::STORAGE && destination == Inventory::INVENTORY)
    {
        MessageOut outMsg(CSMG_MOVE_FROM_STORAGE);
        outMsg.writeInt16(slot + STORAGE_OFFSET);
        outMsg.writeInt32(amount);
    }
}

size_t InventoryHandler::getSize(int type) const
{
    switch (type)
    {
        case Inventory::INVENTORY:
            return 100;
        case Inventory::STORAGE:
            return 0; // Comes from server after items
        case Inventory::TRADE:
            return 12;
        case GUILD_STORAGE:
            return 0; // Comes from server after items
        default:
            return 0;
    }
}

} // namespace TmwAthena
