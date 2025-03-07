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

#pragma once

#include "eventlistener.h"

#include "net/inventoryhandler.h"

#include "net/manaserv/messagehandler.h"

#include <map>
#include <vector>

namespace ManaServ {

class EquipBackend final : public Equipment::Backend, public EventListener
{
    public:
        EquipBackend();

        Item *getEquipment(int slotIndex) const override;
        std::string getSlotName(int slotIndex) const override;
        void clear() override;

        void equip(int inventorySlot, int equipmentSlot);
        void unequip(int inventorySlot);

        void event(Event::Channel channel, const Event &event) override;

        int getSlotNumber() const override
        { return mSlots.size(); }

        unsigned int getVisibleSlotsNumber() const
        { return mVisibleSlots; }

        void triggerUnequip(int slotIndex) const override;

        bool isWeaponSlot(int slotTypeId) const;
        bool isAmmoSlot(int slotTypeId) const;

        Position getBoxPosition(unsigned int slotIndex) const;

        const std::string &getBoxBackground(unsigned int slotIndex) const;

    private:
        void readEquipFile() override;

        void readBoxNode(XML::Node slotNode);

        struct Slot {
            // Generic info
            std::string name;

            int inventorySlot = 0;

            // Manaserv specific info

            // Used to know which (server-side) slot id it is.
            unsigned int slotTypeId = 0;
            // Static part
            // The sub id is used to know in which order the slots are
            // when the slotType has more than one slot capacity:
            // I.e.: capacity = 6, subId will be between 1 and 6
            // for each slots in the map.
            // This is used to sort the multimap along with the slot id.
            unsigned int subId = 0;

            // Tell whether the slot is a weapon slot
            bool weaponSlot = false;

            // Tell whether the slot is an ammo slot
            bool ammoSlot = false;
         };

        unsigned int mVisibleSlots;

        // slot client index, slot info
        std::map<unsigned int, Slot> mSlots;
        std::vector<Position> mBoxesPositions;
        std::vector<std::string> mBoxesBackgroundFile;
};

class InventoryHandler final : public MessageHandler, Net::InventoryHandler,
        public EventListener
{
    public:
        InventoryHandler();

        void handleMessage(MessageIn &msg) override;

        void event(Event::Channel channel, const Event &event) override;

        size_t getSize(int type) const override;

        bool isWeaponSlot(unsigned int slotTypeId) const override
        { return mEquipBackend.isWeaponSlot(slotTypeId); }

        bool isAmmoSlot(unsigned int slotTypeId) const override
        { return mEquipBackend.isAmmoSlot(slotTypeId); }

        unsigned int getVisibleSlotsNumber() const override
        { return mEquipBackend.getVisibleSlotsNumber(); }

        Equipment::Backend *getEquipmentBackend() override
        { return &mEquipBackend; }

        Position getBoxPosition(unsigned int slotIndex) const override
        { return mEquipBackend.getBoxPosition(slotIndex); }

        const std::string& getBoxBackground(unsigned int slotIndex) const override
        { return mEquipBackend.getBoxBackground(slotIndex); }

    private:
        EquipBackend mEquipBackend;
};

} // namespace ManaServ
