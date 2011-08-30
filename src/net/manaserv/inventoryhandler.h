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

#ifndef NET_MANASERV_INVENTORYHANDLER_H
#define NET_MANASERV_INVENTORYHANDLER_H

#include "equipment.h"
#include "eventlistener.h"

#include "net/inventoryhandler.h"

#include "net/manaserv/messagehandler.h"

#include <vector>

namespace ManaServ {

class EquipBackend : public Equipment::Backend, public EventListener
{
    public:
        EquipBackend();

        ~EquipBackend();

        Item *getEquipment(int slotIndex) const;
        std::string getSlotName(int slotIndex) const;
        void clear();

        void equip(int itemId, int slotTypeId, int amountUsed = 1,
                   int itemInstance = 0);
        void unequip(int slotTypeId);

        void event(Event::Channel channel, const Event &event);

        int getSlotNumber() const
        { return mSlots.size(); }

        unsigned int getVisibleSlotsNumber() const
        { return mVisibleSlots; }

        void triggerUnequip(int slotIndex) const;

        bool isWeaponSlot(int slotTypeId) const;
        bool isAmmoSlot(int slotTypeId) const;

        Position getBoxPosition(unsigned int slotIndex) const;

    private:
        void readEquipFile();

        void readBoxNode(xmlNodePtr slotNode);

        struct Slot {
            Slot():
                item(0),
                slotTypeId(0),
                subId(0),
                itemInstance(0),
                weaponSlot(false),
                ammoSlot(false)
            {}

            // Generic info
            std::string name;

            // The Item reference, used for graphical representation
            // and info.
            Item *item;

            // Manaserv specific info

            // Used to know which (server-side) slot id it is.
            unsigned int slotTypeId;
            // Static part
            // The sub id is used to know in which order the slots are
            // when the slotType has more than one slot capacity:
            // I.e.: capacity = 6, subId will be between 1 and 6
            // for each slots in the map.
            // This is used to sort the multimap along with the slot id.
            unsigned int subId;

            // This is the (per character) unique item Id, used especially when
            // equipping the same item multiple times on the same slot type.
            unsigned int itemInstance;

            // Tell whether the slot is a weapon slot
            bool weaponSlot;

            // Tell whether the slot is an ammo slot
            bool ammoSlot;
         };

        unsigned int mVisibleSlots;

        // slot client index, slot info
        typedef std::map<unsigned int, Slot> Slots;
        Slots mSlots;
        std::vector<Position> mBoxesPositions;
};

class InventoryHandler : public MessageHandler, Net::InventoryHandler,
        public EventListener
{
    public:
        InventoryHandler();

        void handleMessage(Net::MessageIn &msg);

        void event(Event::Channel channel, const Event &event);

        bool canSplit(const Item *item);

        size_t getSize(int type) const;

        bool isWeaponSlot(unsigned int slotTypeId) const
        { return mEquipBackend.isWeaponSlot(slotTypeId); }

        bool isAmmoSlot(unsigned int slotTypeId) const
        { return mEquipBackend.isAmmoSlot(slotTypeId); }

        unsigned int getVisibleSlotsNumber() const
        { return mEquipBackend.getVisibleSlotsNumber(); }

        Position getBoxPosition(unsigned int slotIndex) const
        { return mEquipBackend.getBoxPosition(slotIndex); }

    private:
        EquipBackend mEquipBackend;
};

} // namespace ManaServ

#endif // NET_MANASERV_INVENTORYHANDLER_H
