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

#include "../graphic/graphic.h"
#include "equipment.h"
#include "../resources/resourcemanager.h"
#include "../resources/image.h"

EquipmentWindow::EquipmentWindow():
    Window("Equipment")
{
    setSize(60, 200);
    setPosition(40, 40);
    
    ResourceManager *resman = ResourceManager::getInstance();
    Image *itemImg = resman->getImage("core/graphics/sprites/items.png");
    if (!itemImg) error("Unable to load items.png");
    itemset = new Spriteset(itemImg, 20, 20);
    
    for (int i = 0; i < 10; i++ ) {
        equipments[i] = 0;
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
        if (equipments[i] > 0) {
            itemset->spriteset[equipments[i] - 501]->draw(screen,
                        x + 22, y + 24 * i + 20);
        }
        graphics->setColor(gcn::Color(0, 0, 0));
        graphics->drawRectangle(gcn::Rectangle(22, 24 * i + 20, 20, 20));
    }
}

void EquipmentWindow::action(const std::string &eventId)
{
}

void EquipmentWindow::addEquipment(int index, int id) {
    equipments[index] = id;
}

void EquipmentWindow::removeEquipment(int index) {
    equipments[index] = 0;
}

