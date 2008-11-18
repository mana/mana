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
 */

#include "itemcontainer.h"

#include <guichan/mouseinput.hpp>
#include <guichan/selectionlistener.hpp>

#include "../graphics.h"
#include "../inventory.h"
#include "../item.h"
#include "../itemshortcut.h"
#include "../localplayer.h"
#include "../log.h"

#include "../resources/image.h"
#include "../resources/iteminfo.h"
#include "../resources/resourcemanager.h"

#include "../utils/tostring.h"

const int ItemContainer::gridWidth = 36;  // item icon width + 4
const int ItemContainer::gridHeight = 42; // item icon height + 10

ItemContainer::ItemContainer(Inventory *inventory, int offset):
    mInventory(inventory),
    mSelectedItem(NULL),
    mOffset(offset)
{
    ResourceManager *resman = ResourceManager::getInstance();

    mSelImg = resman->getImage("graphics/gui/selection.png");
    if (!mSelImg) logger->error("Unable to load selection.png");

    mMaxItems = mInventory->getLastUsedSlot() - 1; // Count from 0, usage from 2

    addMouseListener(this);
    addWidgetListener(this);
}

ItemContainer::~ItemContainer()
{
    mSelImg->decRef();
}

void
ItemContainer::logic()
{
    gcn::Widget::logic();

    int i = mInventory->getLastUsedSlot() - 1; // Count from 0, usage from 2

    if (i != mMaxItems)
    {
        mMaxItems = i;
        recalculateHeight();
    }
}

void
ItemContainer::draw(gcn::Graphics *graphics)
{
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
     * mOffset is used to compensate for some weirdness that eAthena inherited from
     * Ragnarok Online.  Inventory slots and cart slots are +2 from their actual index,
     * while storage slots are +1.
     */
    for (int i = mOffset; i < mInventory->getSize(); i++)
    {
        Item *item = mInventory->getItem(i);

        if (!item || item->getQuantity() <= 0)
            continue;

        int itemX = ((i - 2) % columns) * gridWidth;
        int itemY = ((i - 2) / columns) * gridHeight;

        // Draw selection image below selected item
        if (mSelectedItem == item)
        {
            static_cast<Graphics*>(graphics)->drawImage(
                    mSelImg, itemX, itemY);
        }

        // Draw item icon
        Image* image = item->getImage();
        if (image)
        {
            static_cast<Graphics*>(graphics)->drawImage(
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

void ItemContainer::widgetResized(const gcn::Event &event)
{
    recalculateHeight();
}

void ItemContainer::recalculateHeight()
{
    int cols = getWidth() / gridWidth;

    if (cols < 1)
        cols = 1;

    const int rows = (mMaxItems / cols) + (mMaxItems % cols > 0 ? 1 : 0);
    const int height = rows * gridHeight + 8;
    if (height != getHeight())
        setHeight(height);
}

Item*
ItemContainer::getSelectedItem() const
{
    return mSelectedItem;
}

void
ItemContainer::selectNone()
{
    setSelectedItem(NULL);
}

void
ItemContainer::setSelectedItem(Item *item)
{
    if (mSelectedItem != item)
    {
        mSelectedItem = item;
        distributeValueChangedEvent();
    }
}

void
ItemContainer::distributeValueChangedEvent()
{
    gcn::SelectionEvent event(this);
    std::list<gcn::SelectionListener*>::iterator i_end = mListeners.end();
    std::list<gcn::SelectionListener*>::iterator i;

    for (i = mListeners.begin(); i != i_end; ++i)
    {
        (*i)->valueChanged(event);
    }
}

void
ItemContainer::mousePressed(gcn::MouseEvent &event)
{
    int button = event.getButton();

    if (button == gcn::MouseEvent::LEFT || button == gcn::MouseEvent::RIGHT)
    {
        int columns = getWidth() / gridWidth;
        int mx = event.getX();
        int my = event.getY();
        int index = mx / gridWidth + ((my / gridHeight) * columns) + mOffset;

        itemShortcut->setItemSelected(-1);
        // Fix for old server, it should be: if (index >= mMaxItems)
        if (index > mMaxItems + 1)
        {
            setSelectedItem(NULL);
            return;
        }
        Item *item = mInventory->getItem(index);
        setSelectedItem(item);
        if (item && !item->isEquipment())
        {
            itemShortcut->setItemSelected(item->getId());
        }
    }
}
