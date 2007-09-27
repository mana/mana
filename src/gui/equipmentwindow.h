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

#include "window.h"

#include <guichan/actionlistener.hpp>

#include "../equipment.h"

class Equipment;
class Image;

/**
 * Equipment box.
 */
struct EquipBox
{
    int posX;
    int posY;
};

/**
 * Equipment dialog.
 *
 * \ingroup Interface
 */
class EquipmentWindow : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        EquipmentWindow(Equipment *equipment);

        /**
         * Destructor.
         */
        ~EquipmentWindow();

        /**
         * Draws the equipment window.
         */
        void draw(gcn::Graphics *graphics);

        void action(const gcn::ActionEvent &event);

        void mousePressed(gcn::MouseEvent& mouseEvent);

        enum{
            // Equipment rules:
            EQUIP_TORSO_SLOT = 0,
            EQUIP_ARMS_SLOT = 1,
            EQUIP_HEAD_SLOT = 2,
            EQUIP_LEGS_SLOT = 3,
            EQUIP_FEET_SLOT = 4,
            EQUIP_RING1_SLOT = 5,
            EQUIP_RING2_SLOT = 6,
            EQUIP_NECKLACE_SLOT = 7,
            EQUIP_FIGHT1_SLOT = 8,
            EQUIP_FIGHT2_SLOT = 9,
            EQUIP_PROJECTILE_SLOT = 10
        };
    private:
        Equipment *mEquipment;
        gcn::Button *mUnequip;              /**< Button for unequipping. */
        Image *mBackground;                 /**< Background Image. */
        EquipBox mEquipBox[EQUIPMENT_SIZE]; /**< Equipment Boxes. */

        int mSelected;                      /**< Index of selected item. */
};

extern EquipmentWindow *equipmentWindow;

#endif
