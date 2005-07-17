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

#include "itemcontainer.h"
#include "../main.h"
#include "../log.h"
#include "../graphics.h"
#include "../resources/resourcemanager.h"
#include <sstream>

ItemContainer::ItemContainer()
{
    ResourceManager *resman = ResourceManager::getInstance();
    Image *itemImg = resman->getImage("graphics/sprites/items.png");
    if (!itemImg) logger->error("Unable to load items.png");
    itemset = new Spriteset(itemImg, 32, 32);
    itemImg->decRef();

    selImg = resman->getImage("graphics/gui/selection.png");
    if (!selImg) logger->error("Unable to load selection.png");

    selectedItem = 0; // No item selected

    for (int i = 0; i < INVENTORY_SIZE; i++) {
        items[i].setInvIndex(i);
    }

    addMouseListener(this);
}

ItemContainer::~ItemContainer()
{
    delete itemset;
    selImg->decRef();
}

void ItemContainer::draw(gcn::Graphics* graphics)
{
    int gridWidth = itemset->spriteset[0]->getWidth() + 4;
    int gridHeight = itemset->spriteset[0]->getHeight() + 10;
    int w = getWidth();
    int columns = w / gridWidth;
    int x, y;
    getAbsolutePosition(x, y);

    // Have at least 1 column
    if (columns < 1)
    {
        columns = 1;
    }

    // Reset selected item when quantity not above 0 (should probably be made
    // sure somewhere else)
    if (selectedItem && selectedItem->getQuantity() <= 0)
    {
        selectedItem = 0;
    }

    /*
     * eAthena seems to start inventory from the 3rd slot. Still a mystery to
     * us why, make sure not to copy this oddity to our own server.
     */
    for (int i = 2; i < INVENTORY_SIZE; i++)
    {
        if (items[i].getQuantity() > 0)
        {
            int itemX = ((i - 2) % columns) * gridWidth;
            int itemY = ((i - 2) / columns) * gridHeight;

            // Draw selection image below selected item
            if (selectedItem == &items[i])
            {
                dynamic_cast<Graphics*>(graphics)->drawImage(
                        selImg, x + itemX, y + itemY);
            }

            // Draw item icon
            int idx;
            if ((idx = items[i].getInfo()->getImage()) > 0)
            {
                dynamic_cast<Graphics*>(graphics)->drawImage(
                        itemset->spriteset[idx - 1], x + itemX, y + itemY);
            }

            // Draw item caption
            std::stringstream ss;

            if (!items[i].isEquipped()) {
                ss << items[i].getQuantity();
            }
            else {
                ss << "Eq.";
            }

            graphics->drawText(ss.str(),
                    itemX + gridWidth / 2,
                    itemY + gridHeight - 11,
                    gcn::Graphics::CENTER);
        }
    }
}

void ItemContainer::setWidth(int width)
{
    gcn::Widget::setWidth(width);

    int gridWidth = itemset->spriteset[0]->getWidth() + 4;
    int gridHeight = itemset->spriteset[0]->getHeight() + 10;
    int columns = getWidth() / gridWidth;

    if (columns < 1)
    {
        columns = 1;
    }

    setHeight(((INVENTORY_SIZE / columns) +
            (INVENTORY_SIZE % columns > 0 ? 1 : 0)) * gridHeight);
}

int ItemContainer::getIndex(int id)
{
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (items[i].getId() == id) {
            return i;
        }
    }
    return -1;
}

Item* ItemContainer::getItem()
{
    return selectedItem;
}

Item* ItemContainer::getItem(int index)
{
    return &items[index];
}

void ItemContainer::addItem(int index, int id, int quantity, bool equipment)
{
    items[index].setId(id);
    items[index].increaseQuantity(quantity);
    items[index].setEquipment(equipment);
}

int ItemContainer::getFreeSlot()
{
    for (int i = 2; i < INVENTORY_SIZE; i++) {
        if (items[i].getId() == -1) {
            return i;
        }
    }
    return -1;
}

void ItemContainer::resetItems()
{
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        items[i].setId(-1);
        items[i].setQuantity(0);
        items[i].setEquipped(false);
    }
}

void ItemContainer::selectNone()
{
    selectedItem = 0;
}

void ItemContainer::removeItem(int id)
{
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (items[i].getId() == id) {
            items[i].setId(-1);
            items[i].setQuantity(0);
        }
    }
}

void ItemContainer::mousePress(int mx, int my, int button)
{
    int gridWidth = itemset->spriteset[0]->getWidth() + 4;
    int gridHeight = itemset->spriteset[0]->getHeight() + 10;
    int w = getWidth();
    int columns = w / gridWidth;

    if (button == gcn::MouseInput::LEFT)
    {
        int index = mx / gridWidth + ((my / gridHeight) * columns) + 2;

        if (index > INVENTORY_SIZE) {
            index = INVENTORY_SIZE - 1;
        }
        selectedItem = &items[index];
    }
}

int ItemContainer::getNumberOfSlotsUsed()
{
    int NumberOfFilledSlot = 0;
    for (int i = 0; i < INVENTORY_SIZE; i++)
    {
        if (items[i].getId() > -1 || items[i].getQuantity() > 0) 
        {
            NumberOfFilledSlot++;
        }
    }

    return NumberOfFilledSlot;
}
