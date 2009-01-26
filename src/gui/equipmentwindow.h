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
 */

#ifndef EQUIPMENT_WINDOW_H
#define EQUIPMENT_WINDOW_H

#include <guichan/actionlistener.hpp>

#include "window.h"

#include "../equipment.h"

class Inventory;
class PlayerBox;

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

        enum {
            // Equipment rules:
            EQUIP_LEGS_SLOT = 0,
            EQUIP_FIGHT1_SLOT,
            EQUIP_GLOVES_SLOT,
            EQUIP_RING2_SLOT,
            EQUIP_RING1_SLOT,
            EQUIP_FIGHT2_SLOT,
            EQUIP_FEET_SLOT,
            EQUIP_CAPE_SLOT,
            EQUIP_HEAD_SLOT,
            EQUIP_TORSO_SLOT,
            EQUIP_AMMO_SLOT,
            EQUIP_VECTOREND
        };


    private:
        Equipment *mEquipment;
        Inventory *mInventory;
        gcn::Button *mUnequip;                  /**< Button for unequipping. */
        EquipBox mEquipBox[EQUIP_VECTOREND];    /**< Equipment Boxes. */

        PlayerBox *mPlayerBox;

        int mSelected;                          /**< Index of selected item. */
};

extern EquipmentWindow *equipmentWindow;

#endif
