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

#include "playerbox.h"
#include "gui.h"
#include "../main.h"

PlayerBox::PlayerBox():
    hairColor(0),
    hairStyle(0),
    showPlayer(false)
{
    setBorderSize(2);
}

void PlayerBox::draw(gcn::Graphics *graphics)
{
    if (showPlayer)
    {
        int x, y;
        getAbsolutePosition(x, y);

        // Draw character
        playerset->spriteset[0]->draw(gui_bitmap, x - 25, y - 25);

        // Draw his hair
        int hf = hairColor + 40 * (hairStyle);
        hairset->spriteset[hf]->draw(gui_bitmap, x + 37, y + 5);
    }
}

void PlayerBox::drawBorder(gcn::Graphics *graphics)
{
    int x, y, w, h, bs;
    getAbsolutePosition(x, y);
    bs = getBorderSize();
    w = getWidth() + bs * 2;
    h = getHeight() + bs * 2;
    x -= bs;
    y -= bs;

    draw_skinned_rect(gui_bitmap, &gui_skin.textbox.bg, x, y, w, h);
}
