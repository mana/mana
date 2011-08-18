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

#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include <string>

class Item;

class Equipment
{
    public:
        Equipment(): mBackend(0) {}

        ~Equipment() { mBackend = 0; }

        class Backend {
            public:
                virtual Item *getEquipment(int slotIndex) const = 0;
                virtual std::string getSlotName(int slotIndex) const
                { return std::string(); }

                virtual void clear() = 0;
                virtual ~Backend() { }
                virtual int getSlotNumber() const = 0;
                virtual void triggerUnequip(int slotIndex) const = 0;
            private:
                virtual void readEquipFile()
                {}
        };

        /**
         * Get equipment at the given slot.
         */
        Item *getEquipment(int slotIndex) const
        { return mBackend ? mBackend->getEquipment(slotIndex) : 0; }

        const std::string getSlotName(int slotIndex) const
        { return mBackend ? mBackend->getSlotName(slotIndex) : std::string(); }

        int getSlotNumber() const
        { return mBackend ? mBackend->getSlotNumber() : 0; }

        void triggerUnequip(int slotIndex) const
        { if (mBackend) mBackend->triggerUnequip(slotIndex); }

        /**
         * Clears equipment.
         */
        void clear()
        { if (mBackend) mBackend->clear(); }

        /**
         * Set equipment at the given slot.
         */
        void setEquipment(int index, int id, int quantity = 0);

        void setBackend(Backend *backend)
        { mBackend = backend; }

    private:
        Backend *mBackend;
};

#endif
