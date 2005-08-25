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

#include "../being.h"
#include "../graphics.h"

#include "../graphic/imagerect.h"
#include "../graphic/spriteset.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

extern Spriteset *hairset, *playerset;

int PlayerBox::instances = 0;
ImageRect PlayerBox::background;

PlayerBox::PlayerBox():
    hairColor(0),
    hairStyle(0),
    showPlayer(false)
{
    setBorderSize(2);

    if (instances == 0)
    {
        // Load the background skin
        ResourceManager *resman = ResourceManager::getInstance();
        Image *textbox = resman->getImage("graphics/gui/deepbox.png");
        int bggridx[4] = {0, 3, 28, 31};
        int bggridy[4] = {0, 3, 28, 31};
        int a = 0, x, y;

        for (y = 0; y < 3; y++) {
            for (x = 0; x < 3; x++) {
                background.grid[a] = textbox->getSubImage(
                        bggridx[x], bggridy[y],
                        bggridx[x + 1] - bggridx[x] + 1,
                        bggridy[y + 1] - bggridy[y] + 1);
                a++;
            }
        }

        textbox->decRef();
    }

    instances++;
}

PlayerBox::~PlayerBox()
{
    instances--;

    if (instances == 0)
    {
        for (int a = 0; a < 9; a++) {
            delete background.grid[a];
        }
    }
}

void PlayerBox::draw(gcn::Graphics *graphics)
{
    if (!showPlayer) {
        return;
    }

    // Draw character
    dynamic_cast<Graphics*>(graphics)->drawImage(
            playerset->spriteset[0], 23, 23);

    // Draw his hair
    if (hairColor >= 0 && hairStyle >= 0 &&
            hairColor < NR_HAIR_COLORS && hairStyle < NR_HAIR_STYLES)
    {
        int hf = hairColor + 40 * (hairStyle);
        if (hf >= 0 && hf < (int)hairset->spriteset.size()) {
            dynamic_cast<Graphics*>(graphics)->drawImage(
                    hairset->spriteset[hf], 37, 5);
        }
    }
}

void PlayerBox::drawBorder(gcn::Graphics *graphics)
{
    int w, h, bs;
    bs = getBorderSize();
    w = getWidth() + bs * 2;
    h = getHeight() + bs * 2;

    dynamic_cast<Graphics*>(graphics)->drawImageRect(0, 0, w, h, background);
}
