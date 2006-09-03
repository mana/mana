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

#include <guichan/mouseinput.hpp>

#include "selectionlistener.h"

#include "../graphics.h"
#include "../inventory.h"
#include "../item.h"
#include "../log.h"

#include "../resources/image.h"
#include "../resources/iteminfo.h"
#include "../resources/resourcemanager.h"
#include "../resources/spriteset.h"

#include "../utils/tostring.h"

ItemContainer::ItemContainer(Inventory *inventory):
    mInventory(inventory),
    mSelectedItem(NULL)
{
    ResourceManager *resman = ResourceManager::getInstance();

    mSelImg = resman->getImage("graphics/gui/selection.png");
    if (!mSelImg) logger->error("Unable to load selection.png");

    mMaxItems = mInventory->getLastUsedSlot() - 1; // Count from 0, usage from 2

    addMouseListener(this);
}

ItemContainer::~ItemContainer()
{
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
    int gridWidth = 36; //(item icon width + 4)
    int gridHeight = 42; //(item icon height + 10)
    int columns = getWidth() / gridWidth;

    // Have at least 1 column
    if (columns < 1)
    {
        columns = 1;
    }

    // Reset selected item when quantity not above 0 (should probably be made
    // sure somewhere else)
    if (mSelectedItem && mSelectedItem->getQuantity() <= 0)
    {
        selectNone();
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
        Image* image;
        if ((image = item->getInfo().getImage()) != NULL)
        {
            dynamic_cast<Graphics*>(graphics)->drawImage(
                    image, itemX, itemY);
        }

        // Draw item caption
        graphics->drawText(
                (item->isEquipped() ? "Eq." : toString(item->getQuantity())),
                itemX + gridWidth / 2,
                itemY + gridHeight - 11,
                gcn::Graphics::CENTER);
    }
}

void ItemContainer::setWidth(int width)
{
    gcn::Widget::setWidth(width);

    int gridWidth = 36; //item icon width + 4
    int gridHeight = 46; //item icon height + 14
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
    setSelectedItem(NULL);
}

void ItemContainer::setSelectedItem(Item *item)
{
    if (mSelectedItem != item)
    {
        mSelectedItem = item;
        fireSelectionChangedEvent();
    }
}

void ItemContainer::fireSelectionChangedEvent()
{
    SelectionEvent event(this);
    SelectionListeners::iterator i_end = mListeners.end();
    SelectionListeners::iterator i;

    for (i = mListeners.begin(); i != i_end; ++i)
    {
        (*i)->selectionChanged(event);
    }
}

void ItemContainer::mousePress(int mx, int my, int button)
{
    int gridWidth = 36; //(item icon width + 4)
    int gridHeight = 42; //(item icon height + 10)
    int columns = getWidth() / gridWidth;

    if (button == gcn::MouseInput::LEFT || gcn::MouseInput::RIGHT)
    {
        int index = mx / gridWidth + ((my / gridHeight) * columns) + 2;

        if (index > INVENTORY_SIZE) {
            index = INVENTORY_SIZE - 1;
        }
        setSelectedItem(mInventory->getItem(index));
    }
}
