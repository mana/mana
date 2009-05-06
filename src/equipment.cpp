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

#include "equipment.h"
#include "item.h"
#ifdef EATHENA_SUPPORT
#include "inventory.h"
#include "localplayer.h"
#endif

#include <algorithm>

Equipment::Equipment()
#ifdef EATHENA_SUPPORT
    : mArrows(-1)
#endif
{
#ifdef TMWSERV_SUPPORT
    std::fill_n(mEquipment, EQUIPMENT_SIZE, (Item*) 0);
#else
    std::fill_n(mEquipment, EQUIPMENT_SIZE, -1);
#endif
}

#ifdef TMWSERV_SUPPORT

Equipment::~Equipment()
{
    clear();
}

void Equipment::clear()
{
    for (int i = 0; i < EQUIPMENT_SIZE; ++i)
        delete mEquipment[i];

    std::fill_n(mEquipment, EQUIPMENT_SIZE, (Item*) 0);
}

void Equipment::setEquipment(int index, int id)
{
    if (mEquipment[index] && mEquipment[index]->getId() == id)
        return;

    delete mEquipment[index];
    mEquipment[index] = (id > 0) ? new Item(id) : 0;
}

#else

void Equipment::setEquipment(int index, int inventoryIndex)
{
    mEquipment[index] = inventoryIndex;
    Item *item = player_node->getInventory()->getItem(inventoryIndex);
    if (item)
        item->setEquipped(true);
}

void Equipment::removeEquipment(int index)
{
     if (index >= 0 && index < EQUIPMENT_SIZE)
         mEquipment[index] = -1;
}

#endif
