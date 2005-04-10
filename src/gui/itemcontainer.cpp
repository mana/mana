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
#include "itemcontainer.h"
#include "../resources/resourcemanager.h"
#include <sstream>

ItemContainer::ItemContainer()
{
    ResourceManager *resman = ResourceManager::getInstance();
    Image *itemImg = resman->getImage("graphics/sprites/items.png", IMG_ALPHA);
    if (!itemImg) logger.error("Unable to load items.png");
    itemset = new Spriteset(itemImg, 20, 20);

    selImg = resman->getImage("graphics/gui/selection.png", IMG_ALPHA);
    if (!selImg) logger.error("Unable to load items.png");

    selectedItem = -1; /**< No item selected */
    
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        items[i].id = -1;
        items[i].quantity = 0;
        items[i].equipment = false;
        items[i].equipped = false;
    }
}

ItemContainer::~ItemContainer()
{
}

void ItemContainer::draw(gcn::Graphics* graphics)
{
    int x, y, w, h;
    getAbsolutePosition(x, y);
    w = getWidth();
    h = getHeight();

    if (items[selectedItem].quantity <= 0) {
        selectedItem = -1;
    }
    
    if (selectedItem >= 0) {
        int itemX = (((selectedItem - 2) * 24) % (getWidth() - 24));
        int itemY = (((selectedItem - 2) * 24) / (getWidth() - 24)) * 24;
        itemX -= itemX % 24;
    	selImg->draw(screen, x + itemX, y+itemY);
    }

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        int itemX = (((i - 2) * 24) % (getWidth() - 24));
        int itemY = (((i - 2) * 24) / (getWidth() - 24)) * 24;
        itemX -= itemX % 24;
        if (items[i].quantity > 0) {
            if (items[i].id >= 501 && items[i].id <= 1202) {
                itemset->spriteset[items[i].id - 501]->draw(screen,
                        x + itemX,
                        y + itemY);
            }

            std::stringstream ss;
            if(!items[i].equipped)
                ss << items[i].quantity;
            graphics->drawText(ss.str(),
                    itemX + 12,
                    itemY + 16,
                    gcn::Graphics::CENTER);
        }
    }
}

int ItemContainer::getIndex()
{
    return selectedItem;
}

int ItemContainer::getIndex(int id)
{
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (items[i].id == id) {
            return i;
        }
    }
    return -1;
}

int ItemContainer::getId()
{
    if (selectedItem != -1) {
        return items[selectedItem].id;
    }
    else {
        return 0;
    }
}

int ItemContainer::getId(int index)
{
    return items[index].id;
}

int ItemContainer::getQuantity()
{
    if (selectedItem != -1) {
        return items[selectedItem].quantity;
    }
    else {
        return 0;
    }

}

int ItemContainer::getQuantity(int index)
{
    return items[index].quantity;
}

void ItemContainer::addItem(int index, int id, int quantity, bool equipment)
{
    items[index].id = id;
    items[index].quantity += quantity;
    items[index].equipment = equipment;
}

int ItemContainer::getFreeSlot()
{
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (items[i].id == -1) {
            return i;
        }
    }
    return -1;
}

void ItemContainer::resetItems()
{
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        items[i].id = -1;
        items[i].quantity = 0;
    }
}

void ItemContainer::removeItem(int id)
{
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (items[i].id == id) {
            items[i].id = -1;
            items[i].quantity = 0;
        }
    }
}

void ItemContainer::changeQuantity(int index, int quantity)
{
    items[index].quantity = quantity;
}

void ItemContainer::increaseQuantity(int index, int quantity)
{
    items[index].quantity += quantity;
}

void ItemContainer::mousePress(int mx, int my, int button)
{
    if (button == gcn::MouseInput::LEFT)
        selectedItem = ((mx + 48) / 24) + ((my / 24) * (getWidth() / 24));
    if (selectedItem > INVENTORY_SIZE)
        selectedItem = INVENTORY_SIZE;
}

void ItemContainer::_mouseInputMessage(const gcn::MouseInput &mouseInput)
{
    if (mouseInput.getButton() == gcn::MouseInput::LEFT) {
        gcn::Widget::_mouseInputMessage(mouseInput);
        mousePress(mouseInput.x, mouseInput.y, mouseInput.getButton());
    }
}

bool ItemContainer::isEquipment(int index)
{
    return items[index].equipment;
}

bool ItemContainer::isEquipped(int index)
{
    return items[index].equipped;
}

void ItemContainer::setEquipped(int index, bool equipped)
{
    items[index].equipped = equipped;
}

void ItemContainer::setEquipment(int index, bool equipment)
{
    items[index].equipment = equipment;
}
