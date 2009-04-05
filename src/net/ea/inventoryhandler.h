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

#ifndef NET_EA_INVENTORYHANDLER_H
#define NET_EA_INVENTORYHANDLER_H

#include "net/inventoryhandler.h"
#include "net/messagehandler.h"
#include "net/net.h"

namespace EAthena {

class InventoryHandler : public MessageHandler, public Net::InventoryHandler
{
    public:
        InventoryHandler();

        void handleMessage(MessageIn &msg);

        void equipItem(Item *item);

        void unequipItem(Item *item);

        void useItem(Item *item);

        void dropItem(Item *item, int amount);

        void splitItem(Item *item, int amount);

        void openStorage();

        void closeStorage();

        void moveItem(StorageType source, int slot, int amount,
                      StorageType destination);
};

} // namespace EAthena

#endif // NET_EA_INVENTORYHANDLER_H
