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

#ifndef NET_EA_INVENTORYHANDLER_H
#define NET_EA_INVENTORYHANDLER_H

#include "equipment.h"
#include "inventory.h"
#include "localplayer.h"

#include "net/inventoryhandler.h"
#include "net/net.h"

#include "net/ea/messagehandler.h"

namespace EAthena {

class EquipBackend : public Equipment::Backend {
    public:
        EquipBackend()
        {
            memset(mEquipment, -1, sizeof(mEquipment));
        }

        Item *getEquipment(int index) const
        {
            int invyIndex = mEquipment[index];
            if (invyIndex == -1)
            {
                return NULL;
            }
            return player_node->getInventory()->getItem(invyIndex);
        }

        void clear()
        {
            for (int i = 0; i < EQUIPMENT_SIZE; i++)
            {
                if (mEquipment[i] != -1)
                {
                    Item* item = player_node->getInventory()->getItem(i);
                    if (item)
                    {
                        item->setEquipped(false);
                    }
                }

                mEquipment[i] = -1;
            }
        }

        void setEquipment(int index, int inventoryIndex)
        {
            // Unequip existing item
            Item* item = player_node->getInventory()->getItem(mEquipment[index]);
            if (item)
            {
                item->setEquipped(false);
            }

            mEquipment[index] = inventoryIndex;

            item = player_node->getInventory()->getItem(inventoryIndex);
            if (item)
            {
                item->setEquipped(true);
            }
        }

    private:
        int mEquipment[EQUIPMENT_SIZE];
};

class InventoryHandler : public MessageHandler, public Net::InventoryHandler
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
        EquipBackend mEquips;
};

} // namespace EAthena

#endif // NET_EA_INVENTORYHANDLER_H
