/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#ifndef _EQUIPMENT_H
#define _EQUIPMENT_H

#include "item.h"

#define EQUIPMENT_SIZE 10

class Equipment
{
    public:
        static Equipment* getInstance();

        Item* getEquipment(int index);
        void setEquipment(int index, Item *item);

        void removeEquipment(int index);
        void removeEquipment(Item *item);

        Item* getArrows();
        void setArrows(Item *arrows);

    protected:
        Equipment();
        ~Equipment();

        Item *equipment[EQUIPMENT_SIZE];
        Item *arrows;

    private:
        static Equipment *instance;
};

inline Equipment *Equipment::getInstance()
{
    if (!instance)
        instance = new Equipment();

    return instance;
}

inline Item* Equipment::getEquipment(int index)
{
    return equipment[index];
}

inline void Equipment::setEquipment(int index, Item *item)
{
    equipment[index] = item;
}

inline void Equipment::removeEquipment(int index)
{
    equipment[index] = 0;
}

inline void Equipment::removeEquipment(Item *item)
{
    for (int i = 0; i < EQUIPMENT_SIZE; i++) {
        if (equipment[i] == item) {
            equipment[i] = 0;
            break;
        }
    }
}

inline Item* Equipment::getArrows()
{
    return arrows;
}

inline void Equipment::setArrows(Item *arrows)
{
    this->arrows = arrows;
}

#endif
