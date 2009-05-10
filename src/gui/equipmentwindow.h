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

#ifndef EQUIPMENTWINDOW_H
#define EQUIPMENTWINDOW_H

#include "gui/widgets/window.h"

#include "guichanfwd.h"

#include <guichan/actionlistener.hpp>

class Equipment;
class Inventory;
class Item;
class ItemPopup;

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
#ifdef TMWSERV_SUPPORT
        EquipmentWindow(Equipment *equipment);
#else
        EquipmentWindow();
#endif

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

#ifdef TMWSERV_SUPPORT
        enum EquipmentSlots
        {
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
            EQUIP_PROJECTILE_SLOT = 10,
            EQUIP_VECTOREND
        };
#else
        enum EquipmentSlots
        {
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
#endif

    private:
        void mouseExited(gcn::MouseEvent &event);
        void mouseMoved(gcn::MouseEvent &event);

        Item *getItem(int x, int y) const;

        void setSelected(int index);

        Equipment *mEquipment;
#ifdef EATHENA_SUPPORT
        Inventory *mInventory;
#endif

        /**
         * Equipment box.
         */
        struct EquipBox
        {
            int posX;
            int posY;
        };

        EquipBox mEquipBox[EQUIP_VECTOREND];    /**< Equipment Boxes. */

        ItemPopup *mItemPopup;
        gcn::Button *mUnequip;

        int mSelected;                          /**< Index of selected item. */
};

extern EquipmentWindow *equipmentWindow;

#endif
