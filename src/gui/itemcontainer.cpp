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

#include <sstream>

#include <guichan/mouseinput.hpp>

#include "../graphics.h"
#include "../inventory.h"
#include "../item.h"
#include "../log.h"

#include "../graphic/spriteset.h"

#include "../resources/image.h"
#include "../resources/iteminfo.h"
#include "../resources/resourcemanager.h"

ItemContainer::ItemContainer(Inventory *inventory):
    mInventory(inventory)
{
    ResourceManager *resman = ResourceManager::getInstance();
    mItemset = resman->createSpriteset("graphics/sprites/items.png", 32, 32);
    if (!mItemset) logger->error("Unable to load items.png");

    mSelImg = resman->getImage("graphics/gui/selection.png");
    if (!mSelImg) logger->error("Unable to load selection.png");

    mSelectedItem = 0; // No item selected
    mMaxItems = mInventory->getLastUsedSlot() - 1; // Count from 0, usage from 2

    addMouseListener(this);
}

ItemContainer::~ItemContainer()
{
    delete mItemset;
    mSelImg->decRef();
}

void ItemContainer::logic()
{
    gcn::Widget::logic();

    int i = mInventory->getLastUsedSlot() - 1; // Count from 0, usage from 2

    if (i != mMaxItems) {
        mMaxItems = i;
        setWidth(getWidth());
    }
}

void ItemContainer::draw(gcn::Graphics* graphics)
{
    int gridWidth = mItemset->get(0)->getWidth() + 4;
    int gridHeight = mItemset->get(0)->getHeight() + 10;
    int w = getWidth();
    int columns = w / gridWidth;

    // Have at least 1 column
    if (columns < 1)
    {
        columns = 1;
    }

    // Reset selected item when quantity not above 0 (should probably be made
    // sure somewhere else)
    if (mSelectedItem && mSelectedItem->getQuantity() <= 0)
    {
        mSelectedItem = 0;
    }

    /*
     * eAthena seems to start inventory from the 3rd slot. Still a mystery to
     * us why, make sure not to copy this oddity to our own server.
     */
    for (int i = 2; i < INVENTORY_SIZE; i++)
    {
        Item *item = mInventory->getItem(i);

        if (item->getQuantity() <= 0) {
            continue;
        }

        int itemX = ((i - 2) % columns) * gridWidth;
        int itemY = ((i - 2) / columns) * gridHeight;

        // Draw selection image below selected item
        if (mSelectedItem == item)
        {
            dynamic_cast<Graphics*>(graphics)->drawImage(
                    mSelImg, itemX, itemY);
        }

        // Draw item icon
        int idx;
        if ((idx = item->getInfo()->getImage()) > 0)
        {
            dynamic_cast<Graphics*>(graphics)->drawImage(
                    mItemset->get(idx - 1), itemX, itemY);
        }

        // Draw item caption
        std::stringstream ss;

        if (!item->isEquipped()) {
            ss << item->getQuantity();
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

void ItemContainer::setWidth(int width)
{
    gcn::Widget::setWidth(width);

    int gridWidth = mItemset->get(0)->getWidth() + 4;
    int gridHeight = mItemset->get(0)->getHeight() + 14;
    int columns = getWidth() / gridWidth;

    if (columns < 1)
    {
        columns = 1;
    }

    setHeight(((mMaxItems / columns) +
            (mMaxItems % columns > 0 ? 1 : 0)) * gridHeight);
}

Item* ItemContainer::getItem()
{
    return mSelectedItem;
}

void ItemContainer::selectNone()
{
    mSelectedItem = 0;
}

void ItemContainer::mousePress(int mx, int my, int button)
{
    int gridWidth = mItemset->get(0)->getWidth() + 4;
    int gridHeight = mItemset->get(0)->getHeight() + 10;
    int w = getWidth();
    int columns = w / gridWidth;

    if (button == gcn::MouseInput::LEFT || gcn::MouseInput::RIGHT)
    {
        int index = mx / gridWidth + ((my / gridHeight) * columns) + 2;

        if (index > INVENTORY_SIZE) {
            index = INVENTORY_SIZE - 1;
        }
        mSelectedItem = mInventory->getItem(index);
    }
}
