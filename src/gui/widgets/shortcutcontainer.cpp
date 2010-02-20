/*
 *  The Mana Client
 *  Copyright (C) 2007-2010  The Mana World Development Team
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

#include "configuration.h"

#include "resources/image.h"

#include "utils/stringutils.h"

float ShortcutContainer::mAlpha = 1.0;

ShortcutContainer::ShortcutContainer():
    mGridWidth(1),
    mGridHeight(1)
{
}

void ShortcutContainer::widgetResized(const gcn::Event &event)
{
    mGridWidth = getWidth() / mBoxWidth;

    if (mGridWidth < 1)
        mGridWidth = 1;

    mGridHeight = mMaxItems / mGridWidth;

    if (mMaxItems % mGridWidth != 0 || mGridHeight < 1)
        ++mGridHeight;

    setHeight(mGridHeight * mBoxHeight);
}

int ShortcutContainer::getIndexFromGrid(int pointX, int pointY) const
{
    const gcn::Rectangle tRect = gcn::Rectangle(0, 0, mGridWidth * mBoxWidth,
                                                mGridHeight * mBoxHeight);

    int index = ((pointY / mBoxHeight) * mGridWidth) + pointX / mBoxWidth;

    if (!tRect.isPointInRect(pointX, pointY) || index >= mMaxItems)
        index = -1;

    return index;
}
