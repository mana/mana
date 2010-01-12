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

#ifndef NET_MANASERV_INVENTORYHANDLER_H
#define NET_MANASERV_INVENTORYHANDLER_H

#include "equipment.h"

#include "net/inventoryhandler.h"

#include "net/manaserv/messagehandler.h"

namespace ManaServ {

class EquipBackend : public Equipment::Backend {
    public:
        EquipBackend()
        { memset(mEquipment, 0, sizeof(mEquipment)); }

        Item *getEquipment(int index) const
        { return mEquipment[index]; }

        void clear()
        {
            for (int i = 0; i < EQUIPMENT_SIZE; ++i)
                delete mEquipment[i];

            std::fill_n(mEquipment, EQUIPMENT_SIZE, (Item*) 0);
        }

        void setEquipment(int index, int id, int quantity = 0)
        {
            if (mEquipment[index] && mEquipment[index]->getId() == id)
                return;

            delete mEquipment[index];
            mEquipment[index] = (id > 0) ? new Item(id, quantity) : 0;

            if (mEquipment[index])
            {
                mEquipment[index]->setInvIndex(index);
                mEquipment[index]->setEquipped(true);
                mEquipment[index]->setInEquipment(true);
            }
        }

    private:
        Item *mEquipment[EQUIPMENT_SIZE];
};

class InventoryHandler : public MessageHandler, Net::InventoryHandler
{
    public:
        InventoryHandler();

        void handleMessage(Net::MessageIn &msg);

        void equipItem(const Item *item);

        void unequipItem(const Item *item);

        void useItem(const Item *item);

        void dropItem(const Item *item, int amount);

        bool canSplit(const Item *item);

        void splitItem(const Item *item, int amount);

        void moveItem(int oldIndex, int newIndex);

        void openStorage(StorageType type);

        void closeStorage(StorageType type);

        void moveItem(StorageType source, int slot, int amount,
                              StorageType destination);

        size_t getSize(StorageType type) const;

    private:
        EquipBackend mEqiups;
};

} // namespace ManaServ

#endif // NET_MANASERV_INVENTORYHANDLER_H
