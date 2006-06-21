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

#include "../utils/dtor.h"

extern std::vector<Spriteset *> hairset;
extern Spriteset *playerset[2];

int PlayerBox::instances = 0;
ImageRect PlayerBox::background;

PlayerBox::PlayerBox(unsigned char sex):
    mHairColor(0),
    mHairStyle(0),
    mSex(sex),
    mShowPlayer(false)
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
        for_each(background.grid, background.grid + 9, dtor<Image*>());
    }
}

void PlayerBox::draw(gcn::Graphics *graphics)
{
    if (!mShowPlayer) {
        return;
    }

    // Draw character
    dynamic_cast<Graphics*>(graphics)->drawImage(
            playerset[mSex]->get(0), 23, 12);

    // Draw his hair
    if (mHairColor >= 0 && mHairStyle >= 0 &&
            mHairColor < NR_HAIR_COLORS && mHairStyle < NR_HAIR_STYLES)
    {
        int hf = 9 * mHairColor;
        if (hf >= 0 && hf < (int)hairset[mHairStyle]->size()) {
            dynamic_cast<Graphics*>(graphics)->drawImage(
                    hairset[mHairStyle]->get(hf), 35, 7);
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
