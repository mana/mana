/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "gui/widgets/playerbox.h"

#include "being.h"
#include "graphics.h"

PlayerBox::PlayerBox(const Being *being)
    : mBeing(being)
{
}

void PlayerBox::draw(gcn::Graphics *graphics)
{
    ScrollArea::draw(graphics);

    if (mBeing)
    {
        // Draw character
        const int x = getWidth() / 2;
        const int y = (getHeight() + mBeing->getHeight()) / 2 - 12;
        mBeing->drawSpriteAt(static_cast<Graphics*>(graphics), x, y);
    }
}
