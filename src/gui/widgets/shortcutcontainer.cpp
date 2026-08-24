/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
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

#include "gui/widgets/shortcutcontainer.h"

#include "inventory.h"
#include "item.h"
#include "playerinfo.h"

#include "gui/gui.h"

#include "resources/theme.h"

ShortcutContainer::ShortcutContainer()
{
    addMouseListener(this);
    addWidgetListener(this);

    auto &skin = gui->getTheme()->getSkin(SkinType::ShortcutBox);
    mBoxWidth = skin.width;
    mBoxHeight = skin.height;
}

ShortcutContainer::~ShortcutContainer() = default;

void ShortcutContainer::setMaxItems(int maxItems)
{
    if (mMaxItems == maxItems)
        return;

    mMaxItems = maxItems;
    updateGrid();
}

void ShortcutContainer::widgetResized(const gcn::Event &event)
{
    updateGrid();
}

void ShortcutContainer::updateGrid()
{
    mGridWidth = getWidth() / mBoxWidth;

    if (mGridWidth < 1)
        mGridWidth = 1;

    mGridHeight = mMaxItems / mGridWidth;

    if (mMaxItems % mGridWidth != 0 || mGridHeight < 1)
        ++mGridHeight;

    setHeight(mGridHeight * mBoxHeight);
}

Item *ShortcutContainer::getDisplayItem(int itemId)
{
    if (Item *item = PlayerInfo::getInventory()->findItem(itemId))
        return item;

    auto i = mFallbackItems.find(itemId);
    if (i == mFallbackItems.end())
    {
        i = mFallbackItems.emplace(
                itemId, std::make_unique<Item>(itemId)).first;
    }

    return i->second.get();
}

void ShortcutContainer::cleanupFallbackItems()
{
    auto *inventory = PlayerInfo::getInventory();
    for (auto i = mFallbackItems.begin(); i != mFallbackItems.end();)
    {
        const int itemId = i->first;

        if (inventory->findItem(itemId))
        {
            i = mFallbackItems.erase(i);
            continue;
        }

        bool stillReferenced = false;
        for (int slot = 0; slot < mMaxItems; ++slot)
        {
            if (getSlotItemId(slot) == itemId)
            {
                stillReferenced = true;
                break;
            }
        }

        if (!stillReferenced)
            i = mFallbackItems.erase(i);
        else
            ++i;
    }
}

int ShortcutContainer::getIndexFromGrid(int pointX, int pointY) const
{
    const gcn::Rectangle tRect(0, 0, mGridWidth * mBoxWidth,
                               mGridHeight * mBoxHeight);

    int index = ((pointY / mBoxHeight) * mGridWidth) + pointX / mBoxWidth;

    if (!tRect.isPointInRect(pointX, pointY) || index >= mMaxItems)
        index = -1;

    return index;
}

int ShortcutContainer::getIndexFromAbsolute(int absX, int absY) const
{
    int widgetX = 0;
    int widgetY = 0;
    getAbsolutePosition(widgetX, widgetY);

    return getIndexFromGrid(absX - widgetX, absY - widgetY);
}

bool ShortcutContainer::isSlotDragged(int index) const
{
    const auto *drag = gui->getActiveDrag();
    return drag &&
           drag->source == this &&
           drag->sourceIndex == index;
}

void ShortcutContainer::dragFinished(const Drag &drag, DragResult result)
{
    if (result == DragResult::Ignored &&
        drag.source == this &&
        drag.sourceIndex >= 0 &&
        drag.sourceIndex < mMaxItems)
    {
        removeSlot(drag.sourceIndex);
    }

    mClickedIndex = -1;
}
