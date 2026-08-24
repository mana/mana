/*
 *  The Mana Client
 *  Copyright (C) 2009-2026  The Mana Developers
 *
 *  This file is part of The Mana Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/widgets/outfitcontainer.h"

#include "graphics.h"
#include "item.h"

#include "gui/gui.h"
#include "gui/itempopup.h"
#include "gui/outfitwindow.h"
#include "gui/viewport.h"

#include "resources/image.h"

OutfitContainer::OutfitContainer(OutfitWindow *outfitWindow)
    : mOutfitWindow(outfitWindow)
    , mItemPopup(new ItemPopup)
{
    mBoxWidth = ITEM_ICON_SIZE + 1;
    mBoxHeight = ITEM_ICON_SIZE + 1;

    setMaxItems(mOutfitWindow->getItemCount());
}

OutfitContainer::~OutfitContainer() = default;

void OutfitContainer::draw(gcn::Graphics *graphics)
{
    cleanupFallbackItems();

    auto *g = static_cast<Graphics*>(graphics);

    for (int i = 0; i < mMaxItems; i++)
    {
        const int itemX = (i % mGridWidth) * mBoxWidth;
        const int itemY = (i / mGridWidth) * mBoxHeight;
        const gcn::Rectangle box(itemX, itemY, ITEM_ICON_SIZE, ITEM_ICON_SIZE);

        graphics->setColor(gcn::Color(0, 0, 0, 64));
        graphics->drawRectangle(box);
        graphics->setColor(gcn::Color(255, 255, 255, 32));
        graphics->fillRectangle(box);

        const int itemId = getSlotItemId(i);
        if (itemId < 0)
            continue;

        Item *item = getDisplayItem(itemId);

        // Items which are currently not in the inventory are shown faded
        const bool isMissing = item->getQuantity() == 0;
        const float alpha = isMissing ? 0.25f
                                      : (isSlotDragged(i) ? 0.5f : 1.0f);

        if (Image *image = item->getImage())
        {
            image->setAlpha(alpha);
            g->drawImage(image, itemX, itemY);
        }
    }
}

void OutfitContainer::mouseDragged(gcn::MouseEvent &event)
{
    if (event.getButton() != gcn::MouseEvent::LEFT)
        return;
    if (gui->getActiveDrag() || mClickedIndex == -1)
        return;

    const int itemId = getSlotItemId(mClickedIndex);
    if (itemId < 0)
        return;

    mItemPopup->setVisible(false);
    gui->startDrag(Drag::fromOutfit(getDisplayItem(itemId),
                                    this, mClickedIndex));
}

void OutfitContainer::mousePressed(gcn::MouseEvent &event)
{
    mClickedIndex = getIndexFromGrid(event.getX(), event.getY());

    if (event.getButton() != gcn::MouseEvent::LEFT)
        return;

    // Add the item which was selected in the inventory window
    if (mOutfitWindow->isItemSelected())
    {
        mOutfitWindow->insertItem(mOutfitWindow->getItemSelected(),
                                  mClickedIndex);
        mOutfitWindow->setItemSelected(-1);
        mClickedIndex = -1;
    }
}

void OutfitContainer::mouseReleased(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        mOutfitWindow->setItemSelected(-1);
        mClickedIndex = -1;
    }
}

void OutfitContainer::mouseMoved(gcn::MouseEvent &event)
{
    const int index = getIndexFromGrid(event.getX(), event.getY());
    const int itemId = index == -1 ? -1 : getSlotItemId(index);

    if (itemId < 0)
    {
        mItemPopup->setVisible(false);
        return;
    }

    mItemPopup->setItem(getDisplayItem(itemId)->getInfo());
    mItemPopup->position(viewport->getMouseX(), viewport->getMouseY());
}

void OutfitContainer::mouseExited(gcn::MouseEvent &event)
{
    mItemPopup->setVisible(false);
}

/**
 * Handles dropping an item on an item slot. Dropping it elsewhere on the
 * outfit window is handled by OutfitWindow.
 */
bool OutfitContainer::handleDrop(const Drag &drag, int absX, int absY)
{
    const Item *item = drag.item.get();
    if (!item || !item->isEquippable())
        return false;

    const int index = getIndexFromAbsolute(absX, absY);
    if (index == -1)
        return false;

    if (drag.source == this)
    {
        mOutfitWindow->moveItem(drag.sourceIndex, index);
        return true;
    }

    return mOutfitWindow->insertItem(item->getId(), index);
}

int OutfitContainer::getSlotItemId(int index) const
{
    return mOutfitWindow->getItem(index);
}

void OutfitContainer::removeSlot(int index)
{
    mOutfitWindow->removeItem(index);
}
