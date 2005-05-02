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

#include "../main.h"
#include "../graphics.h"
#include "equipment.h"
#include "../resources/resourcemanager.h"
#include "../resources/image.h"

#include <sstream>

EquipmentWindow::EquipmentWindow():
    Window("Equipment"), arrows(0)
{
    setContentSize(134, 60);
    setPosition(40, 40);

    ResourceManager *resman = ResourceManager::getInstance();
    Image *itemImg = resman->getImage("graphics/sprites/items.png", IMG_ALPHA);
    if (!itemImg) logger->error("Unable to load items.png");
    itemset = new Spriteset(itemImg, 20, 20);

    for (int i = 0; i < 10; i++ ) {
        equipments[i].id = 0;
        equipments[i].inventoryIndex = -1;
    }
}

EquipmentWindow::~EquipmentWindow()
{
}

void EquipmentWindow::draw(gcn::Graphics *graphics)
{
    int x, y;
    getAbsolutePosition(x, y);
    
    // Draw window graphics
    Window::draw(graphics);
    
    for (int i = 0; i < 8; i++) {
        if (equipments[i].id > 0) {
            itemset->spriteset[itemDb.getItemInfo(
                    equipments[i].id)->getImage() - 1]->draw(
                    screen, x + 24 * (i % 4) + 10, y + 24 * (i / 4) + 25);
        }
        graphics->setColor(gcn::Color(0, 0, 0));
        graphics->drawRectangle(gcn::Rectangle(10 + 24 * (i % 4),
                24 * (i / 4) + 25, 20, 20));
    }
    graphics->setColor(gcn::Color(0, 0, 0));
    graphics->drawRectangle(gcn::Rectangle(110, 25, 20, 20));
    if (arrows) {
        itemset->spriteset[itemDb.getItemInfo(arrows)->getImage() - 1]->draw(
                screen, x + 110, y + 25);
        std::stringstream n;
        n << arrowsNumber;
        graphics->drawText(n.str(), 120, 50,
                    gcn::Graphics::CENTER);
    }
}

void EquipmentWindow::action(const std::string &eventId)
{
}

void EquipmentWindow::addEquipment(int index, int id)
{
    equipments[index].id = id;
}

void EquipmentWindow::removeEquipment(int index)
{
    equipments[index].id = 0;
}

void EquipmentWindow::setInventoryIndex(int index, int inventoryIndex)
{
    equipments[index].inventoryIndex = inventoryIndex;
}

int EquipmentWindow::getInventoryIndex(int index)
{
    return equipments[index].inventoryIndex;
}

void EquipmentWindow::setArrows(int id)
{
    arrows = id;
}

int EquipmentWindow::getArrows()
{
    return arrows;
}
