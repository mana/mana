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

#ifndef EQUIPMENTWINDOW_H
#define EQUIPMENTWINDOW_H

#include "equipment.h"

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

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
        EquipmentWindow(Equipment *equipment);

        ~EquipmentWindow();

        /**
         * Draws the equipment window.
         */
        void draw(gcn::Graphics *graphics);

        void action(const gcn::ActionEvent &event);

        void mousePressed(gcn::MouseEvent& mouseEvent);

        /**
         * Loads the correct amount of displayed equip boxes.
         */
        void loadEquipBoxes();

  protected:
        /**
         * Equipment box.
         */
        struct EquipBox
        {
            int posX;
            int posY;
        };

        EquipBox *mEquipBox; /**< Equipment Boxes. */

        int mSelected; /**< Index of selected item. */
        Equipment *mEquipment;
        int mBoxesNumber; /**< Number of equipment boxes to display */

    private:
        void mouseExited(gcn::MouseEvent &event);
        void mouseMoved(gcn::MouseEvent &event);

        Item *getItem(int x, int y) const;

        void setSelected(int index);

        ItemPopup *mItemPopup;
        gcn::Button *mUnequip;
};

extern EquipmentWindow *equipmentWindow;

#endif // EQUIPMENTWINDOW_H
