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

#include "radiobutton.h"
#include "../resources/resourcemanager.h"

RadioButton::RadioButton(const std::string& caption, const std::string& group,
        bool marked):
    gcn::RadioButton(caption, group, marked)
{
    ResourceManager *resman = ResourceManager::getInstance();
    radioNormal = resman->getImage("graphics/gui/radioout.png");
    radioChecked = resman->getImage("graphics/gui/radioin.png");
    radioDisabled = resman->getImage("graphics/gui/radioout.png");
    radioDisabledChecked = resman->getImage("graphics/gui/radioin.png");
}

void RadioButton::drawBox(gcn::Graphics* graphics)
{
    Image *box = NULL;
    int x, y;

    getAbsolutePosition(x, y);

    if (mMarked) {
        if (isEnabled()) {
            box = radioChecked;
        } else {
            box = radioDisabledChecked;
        }
    } else if (isEnabled()) {
        box = radioNormal;
    } else {
        box = radioDisabled;
    }

    x += 2;
    y += 2;
    if (box != NULL) {
        box->draw(screen, x, y);
    }
}
