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

#include "button.h"
#include "../resources/resourcemanager.h"

Button::Button(const std::string& caption):
    gcn::Button(caption)
{
    setBorderSize(0);

    // Load the skin
    ResourceManager *resman = ResourceManager::getInstance();
    Image *btn[4];
    btn[0] = resman->getImage("core/graphics/gui/button.png");
    btn[1] = resman->getImage("core/graphics/gui/buttonhi.png");
    btn[2] = resman->getImage("core/graphics/gui/buttonpress.png");
    btn[3] = resman->getImage("core/graphics/gui/button_disabled.png");
    int bgridx[4] = {0, 9, 16, 25};
    int bgridy[4] = {0, 4, 19, 24};
    int a, x, y;

    for (int mode = 0; mode < 4; mode++) {
        a = 0;
        for (y = 0; y < 3; y++) {
            for (x = 0; x < 3; x++) {
                button[mode].grid[a] = btn[mode]->getSubImage(
                        bgridx[x], bgridy[y],
                        bgridx[x + 1] - bgridx[x] + 1,
                        bgridy[y + 1] - bgridy[y] + 1);
                a++;
            }
        }
    }
}

void Button::draw(gcn::Graphics* graphics) {
    int mode;

    if (false /*disabled*/) {
        mode = 3;
    }
    else if (isPressed()) {
        mode = 2;
    }
    else if (hasMouse()) {
        mode = 1;
    }
    else {
        mode = 0;
    }

    int x, y;
    getAbsolutePosition(x, y);

    ((Graphics*)graphics)->drawImageRect(x, y, getWidth(), getHeight(),
                                         button[mode]);

    graphics->setColor(getForegroundColor());

    int textX;
    int textY = getHeight() / 2 - getFont()->getHeight() / 2;

    switch (getAlignment()) {
        case gcn::Graphics::LEFT:
            textX = 4;
            break;
        case gcn::Graphics::CENTER:
            textX = getWidth() / 2;
            break;
        case gcn::Graphics::RIGHT:
            textX = getWidth() - 4;
            break;
        default:
            throw GCN_EXCEPTION("Button::draw. Uknown alignment.");
    }

    graphics->setFont(getFont());

    if (isPressed()) {
        graphics->drawText(getCaption(), textX + 1, textY + 1, getAlignment());
    }
    else {
        graphics->drawText(getCaption(), textX, textY, getAlignment());
    }    
}
