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

#include "equipmentwindow.h"

#include "../equipment.h"
#include "../graphics.h"
#include "../item.h"
#include "../log.h"

#include "../resources/iteminfo.h"
#include "../resources/resourcemanager.h"
#include "../resources/spriteset.h"

#include "../utils/tostring.h"

EquipmentWindow::EquipmentWindow(Equipment *equipment):
    Window("Equipment"), mEquipment(equipment)
{
    setWindowName("Equipment");
    setDefaultSize(5, 230, 200, 120);
    loadWindowState();
}

EquipmentWindow::~EquipmentWindow()
{
}

void EquipmentWindow::draw(gcn::Graphics *graphics)
{
    // Draw window graphics
    Window::draw(graphics);

    Item *item;
    Image *image;

    // Rectangles around items are black
    graphics->setColor(gcn::Color(0, 0, 0));

    for (int i = 0; i < EQUIPMENT_SIZE; i++) {
        graphics->drawRectangle(gcn::Rectangle(10 + 36 * (i % 4),
                36 * (i / 4) + 25, 32, 32));

        if (!(item = mEquipment->getEquipment(i))) {
            continue;
        }

        image = item->getInfo().getImage();
        dynamic_cast<Graphics*>(graphics)->drawImage(
                image, 36 * (i % 4) + 10, 36 * (i / 4) + 25);
    }

    graphics->drawRectangle(gcn::Rectangle(160, 25, 32, 32));

    if (!(item = mEquipment->getArrows())) {
        return;
    }

    image = item->getInfo().getImage();

    dynamic_cast<Graphics*>(graphics)->drawImage(image, 160, 25);
    graphics->drawText(toString(item->getQuantity()), 170, 62,
            gcn::Graphics::CENTER);
}
