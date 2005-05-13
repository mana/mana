/*
 *  The Mana World
 *  Copyright 2004-2005 The Mana World Development Team
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

#include "minimap.h"
#include "../being.h"

Minimap::Minimap()
{
    setContentSize(100, 100);
    setPosition(20, 20);
}

void Minimap::draw(gcn::Graphics *graphics)
{
    int x, y;

    getAbsolutePosition(x, y);

    // Transparent background
    graphics->setColor(gcn::Color(52, 149, 210, 120));
    graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));

    // Black border
    graphics->setColor(gcn::Color(0, 0, 0));
    graphics->drawRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));

    // Player dot
    graphics->setColor(gcn::Color(209, 52, 61));
    graphics->fillRectangle(gcn::Rectangle(player_node->x / 2,
            player_node->y / 2, 3, 3));
}
