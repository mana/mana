/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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
#include "resources/image.h"

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

        ~EquipmentWindow() override;

        /**
         * Draws the equipment window.
         */
        void draw(gcn::Graphics *graphics) override;

        void action(const gcn::ActionEvent &event) override;

        void mousePressed(gcn::MouseEvent& mouseEvent) override;

        /**
         * Loads the correct amount of displayed equip boxes.
         */
        void loadEquipBoxes();

        /**
         * Returns the current selected slot or -1 if none.
         */
        int getSelected() const
        { return mSelected; }

  protected:
        /**
         * Equipment box.
         */
        struct EquipBox
        {
            int posX = 0;
            int posY = 0;
            Image *backgroundImage = nullptr;
        };

        std::vector<EquipBox> mBoxes;   /**< Equipment boxes. */

        int mSelected = -1;             /**< Index of selected item. */
        Equipment *mEquipment;

    private:
        void mouseExited(gcn::MouseEvent &event) override;
        void mouseMoved(gcn::MouseEvent &event) override;

        int getBoxIndex(int x, int y) const;
        Item *getItem(int x, int y) const;
        std::string getSlotName(int x, int y) const;

        void setSelected(int index);

        ItemPopup *mItemPopup;
        gcn::Button *mUnequip;
};

extern EquipmentWindow *equipmentWindow;

#endif // EQUIPMENTWINDOW_H
