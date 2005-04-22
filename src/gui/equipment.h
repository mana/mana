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

#ifndef _TMW_EQUIPMENT_H
#define _TMW_EQUIPMENT_H 

#include "../graphic/spriteset.h"
#include "gui.h"
#include "window.h"

typedef struct {
    int id;
    int inventoryIndex;
} EQUIPMENT_HOLDER;

/**
 * Equipment dialog.
 *
 * \ingroup GUI
 */
class EquipmentWindow : public Window, gcn::ActionListener {
    public:
        /**
         * Constructor.
         */
        EquipmentWindow();

        /**
         * Destructor.
         */
        ~EquipmentWindow();

        /**
         * Draws the equipment window.
         */
        void draw(gcn::Graphics *graphics);
        
        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string& eventId);
        
        void addEquipment(int index, int id);
        
        void removeEquipment(int index);
        
        void setInventoryIndex(int index, int inventoryIndex);
        
        int getInventoryIndex(int index);
        
        void setArrows(int id);
        
        EQUIPMENT_HOLDER equipments[10];
        
        int arrowsNumber;
        
    private:
        int arrows;
        Spriteset *itemset;

};

#endif
