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

#include "../graphic/spriteset.h"

#include "../resources/image.h"
#include "../resources/iteminfo.h"
#include "../resources/resourcemanager.h"

#include <sstream>

EquipmentWindow::EquipmentWindow():
    Window("Equipment")
{
    setContentSize(200, 90);
    setPosition(40, 40);
    setName("Equipment");

    ResourceManager *resman = ResourceManager::getInstance();
    Image *itemImg = resman->getImage("graphics/sprites/items.png");
    if (!itemImg) logger->error("Unable to load items.png");
    itemset = new Spriteset(itemImg, 32, 32);
    itemImg->decRef();
}

EquipmentWindow::~EquipmentWindow()
{
    delete itemset;
}

void EquipmentWindow::draw(gcn::Graphics *graphics)
{
    // Draw window graphics
    Window::draw(graphics);

    Equipment *equipment = Equipment::getInstance();
    Item *item;
    Image *image;

    for (int i = 0; i < 8; i++) {
        graphics->setColor(gcn::Color(0, 0, 0));
        graphics->drawRectangle(gcn::Rectangle(10 + 36 * (i % 4),
                36 * (i / 4) + 25, 32, 32));

        if (!(item = equipment->getEquipment(i))) {
            continue;
        }

        image = itemset->spriteset[item->getInfo()->getImage() - 1];
        dynamic_cast<Graphics*>(graphics)->drawImage(
                image, 36 * (i % 4) + 10, 36 * (i / 4) + 25);
    }

    graphics->setColor(gcn::Color(0, 0, 0));
    graphics->drawRectangle(gcn::Rectangle(160, 25, 32, 32));

    if (!(item = equipment->getArrows())) {
        return;
    }

    image = itemset->spriteset[item->getInfo()->getImage() - 1];

    dynamic_cast<Graphics*>(graphics)->drawImage(image, 160, 25);
    std::stringstream n;
    n << item->getQuantity();
    graphics->drawText(n.str(), 170, 62,
            gcn::Graphics::CENTER);
}

void EquipmentWindow::action(const std::string &eventId)
{
}
