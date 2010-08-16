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
#include "listener.h"

#include "net/inventoryhandler.h"

#include "net/manaserv/messagehandler.h"

namespace ManaServ {

class EquipBackend : public Equipment::Backend
{
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

class InventoryHandler : public MessageHandler, Net::InventoryHandler,
        public Mana::Listener
{
    public:
        InventoryHandler();

        void handleMessage(Net::MessageIn &msg);

        void event(const std::string &channel, const Mana::Event &event);

        bool canSplit(const Item *item);

        size_t getSize(int type) const;

    private:
        EquipBackend mEquips;
};

} // namespace ManaServ

#endif // NET_MANASERV_INVENTORYHANDLER_H
