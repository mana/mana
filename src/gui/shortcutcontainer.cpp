/*
 *  The Mana World
 *  Copyright 2007 The Mana World Development Team
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

#include "shortcutcontainer.h"

#include "../graphics.h"
#include "../inventory.h"
#include "../item.h"
#include "../itemshortcut.h"
#include "../keyboardconfig.h"
#include "../localplayer.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

#include "../utils/tostring.h"

ShortcutContainer::ShortcutContainer():
    mGridWidth(1),
    mGridHeight(1)
{
}


void ShortcutContainer::widgetResized(const gcn::Event &event)
{
    mGridWidth = getWidth() / mBoxWidth;
    if (mGridWidth < 1) {
        mGridWidth = 1;
    }

    setHeight((mMaxItems / mGridWidth +
                (mMaxItems % mGridWidth > 0 ? 1 : 0)) * mBoxHeight);

    mGridHeight = getHeight() / mBoxHeight;
    if (mGridHeight < 1) {
        mGridHeight = 1;
    }
}

int
ShortcutContainer::getIndexFromGrid(int pointX, int pointY) const
{
    const gcn::Rectangle tRect = gcn::Rectangle(
        0, 0, mGridWidth * mBoxWidth, mGridHeight * mBoxHeight);
    if (!tRect.isPointInRect(pointX, pointY)) {
        return -1;
    }
    const int index = ((pointY / mBoxHeight) * mGridWidth) +
        pointX / mBoxWidth;
    if (index >= mMaxItems)
    {
        return -1;
    }
    return index;
}
