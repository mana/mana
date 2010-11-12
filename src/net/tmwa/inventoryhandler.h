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

#ifndef NET_TA_INVENTORYHANDLER_H
#define NET_TA_INVENTORYHANDLER_H

#include "equipment.h"
#include "inventory.h"
#include "listener.h"
#include "playerinfo.h"

#include "gui/inventorywindow.h"

#include "net/inventoryhandler.h"
#include "net/net.h"

#include "net/tmwa/messagehandler.h"

#include <list>

namespace TmwAthena {

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
            return PlayerInfo::getInventory()->getItem(invyIndex);
        }

        void clear()
        {
            for (int i = 0; i < EQUIPMENT_SIZE; i++)
            {
                if (mEquipment[i] != -1)
                {
                    Item* item = PlayerInfo::getInventory()->getItem(i);
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
            Item* item = PlayerInfo::getInventory()->getItem(mEquipment[index]);
            if (item)
            {
                item->setEquipped(false);
            }

            mEquipment[index] = inventoryIndex;

            item = PlayerInfo::getInventory()->getItem(inventoryIndex);
            if (item)
            {
                item->setEquipped(true);
            }

            inventoryWindow->updateButtons();
        }

    private:
        int mEquipment[EQUIPMENT_SIZE];
};

/**
 * Used to cache storage data until we get size data for it.
 */
class InventoryItem
{
    public:
        int slot;
        int id;
        int quantity;
        bool equip;

        InventoryItem(int slot, int id, int quantity, bool equip)
        {
            this->slot = slot;
            this->id = id;
            this->quantity = quantity;
            this->equip = equip;
        }
};

typedef std::list<InventoryItem> InventoryItems;

class InventoryHandler : public MessageHandler, public Net::InventoryHandler,
        public Mana::Listener
{
    public:
        enum {
            GUILD_STORAGE = Inventory::TYPE_END,
            CART
        };

        InventoryHandler();

        ~InventoryHandler();

        void handleMessage(Net::MessageIn &msg);

        void event(Channels channel, const Mana::Event &event);

        bool canSplit(const Item *item);

        size_t getSize(int type) const;

    private:
        EquipBackend mEquips;
        InventoryItems mInventoryItems;
        Inventory *mStorage;
        InventoryWindow *mStorageWindow;
};

} // namespace TmwAthena

#endif // NET_TA_INVENTORYHANDLER_H
