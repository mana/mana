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

#include <algorithm>

#include "equipment.h"
#include "item.h"
#include "inventory.h"
#include "localplayer.h"

Equipment::Equipment():
    mArrows(0)
{
    std::fill_n(mEquipment, EQUIPMENT_SIZE, 0);
}

void Equipment::setEquipment(int index, int inventoryIndex)
{
    mEquipment[index] = inventoryIndex;
    Item* item = player_node->getInventory()->getItem(inventoryIndex);
    if (item)
    	item->setEquipped(true);
}

