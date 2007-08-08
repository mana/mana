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

#define BOX_WIDTH 36
#define BOX_HEIGHT 36

#include "equipmentwindow.h"

#include "button.h"

#include "../graphics.h"
#include "../item.h"
#include "../localplayer.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

#include "../utils/gettext.h"

// Positions of the boxes, 2nd dimension is X and Y respectively.
const int boxPosition[][2] = {
    {90, 100},   // EQUIP_TORSO_SLOT
    {135, 60},  // EQUIP_ARMS_SLOT
    {90, 10},    // EQUIP_HEAD_SLOT
    {90, 145},  // EQUIP_LEGS_SLOT
    {90, 190},  // EQUIP_FEET_SLOT
    {35, 105},   // EQUIP_RING1_SLOT
    {145, 105},  // EQUIP_RING2_SLOT
    {90, 55},   // EQUIP_NECKLACE_SLOT
    {20, 150},   // EQUIP_FIGHT1_SLOT
    {160, 150},  // EQUIP_FIGHT2_SLOT
    {45, 60}    // EQUIP_PROJECTILE_SLOT
};

EquipmentWindow::EquipmentWindow(Equipment *equipment):

    Window(_("Equipment")),
    mEquipment(equipment),
    mBackground(NULL),
    mSelected(-1)
{
    setDefaultSize(5, 195, 216, 260);
    loadWindowState("Equipment");

    mUnequip = new Button(_("Unequip"), "unequip", this);
    mUnequip->setPosition(150,235);
    add(mUnequip);

    for (int i = 0; i < EQUIPMENT_SIZE; i++)
    {
        mEquipBox[i].posX = boxPosition[i][0] + getPadding();
        mEquipBox[i].posY = boxPosition[i][1] + getTitleBarHeight();
    }

    ResourceManager *resman = ResourceManager::getInstance();
    mBackground = resman->getImage("graphics/images/equipBackground.png");
    mBackground->setAlpha(0.3);
}

EquipmentWindow::~EquipmentWindow()
{
    mBackground->decRef();
    delete mUnequip;
}

void EquipmentWindow::draw(gcn::Graphics *graphics)
{
    // Draw window graphics
    Window::draw(graphics);

    Graphics *g = static_cast<Graphics*>(graphics);

    if (mBackground)
    {
        g->drawImage(mBackground, getPadding() + 10, 0);
    }

    Window::drawChildren(graphics);

    for (int i = 0; i < EQUIPMENT_SIZE; i++)
    {
        int itemId = mEquipment->getEquipment(i);
        if (itemId)
        {
            // Draw Item.
            Image *image = Item(itemId).getInfo().getImage();
            g->drawImage(image, mEquipBox[i].posX, mEquipBox[i].posY);
        }

        if (i == mSelected)
        {
            // Set color red.
            g->setColor(gcn::Color(255, 0, 0));
        }
        else
        {
            // Set color black.
            g->setColor(gcn::Color(0, 0, 0));
        }
        // Draw box border.
        g->drawRectangle(gcn::Rectangle(mEquipBox[i].posX, mEquipBox[i].posY,
            BOX_WIDTH, BOX_HEIGHT));
    }
}

void EquipmentWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "unequip" && mSelected > -1)
    {
        player_node->unequipItem(mSelected);
        mSelected = -1;
    }
}

void EquipmentWindow::mousePressed(gcn::MouseEvent& mouseEvent)
{
    Window::mousePressed(mouseEvent);

    const int x = mouseEvent.getX();
    const int y = mouseEvent.getY();

    // Checks if any of the presses were in the equip boxes.
    for (int i = 0; i < EQUIPMENT_SIZE; i++)
    {
        gcn::Rectangle tRect(mEquipBox[i].posX, mEquipBox[i].posY,
                             BOX_WIDTH, BOX_HEIGHT);
        if (tRect.isPointInRect(x, y))
        {
            int itemId = mEquipment->getEquipment(i);
            if (itemId)
            {
                mSelected = i;
            }
        }
    }
}
