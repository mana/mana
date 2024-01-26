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

#include "animatedsprite.h"
#include "being.h"
#include "configuration.h"
#include "graphics.h"

#include "resources/image.h"
#include "resources/theme.h"

#include "utils/dtor.h"

int PlayerBox::instances = 0;
float PlayerBox::mAlpha = 1.0;
ImageRect PlayerBox::background;

PlayerBox::PlayerBox(const Being *being):
    mBeing(being)
{
    setFrameSize(2);

    if (instances == 0)
    {
        // Load the background skin
        Image *textbox = Theme::getImageFromTheme("deepbox.png");
        int bggridx[4] = {0, 3, 28, 31};
        int bggridy[4] = {0, 3, 28, 31};
        int a = 0, x, y;

        for (y = 0; y < 3; y++)
        {
            for (x = 0; x < 3; x++)
            {
                background.grid[a] = textbox->getSubImage(
                        bggridx[x], bggridy[y],
                        bggridx[x + 1] - bggridx[x] + 1,
                        bggridy[y + 1] - bggridy[y] + 1);
                background.grid[a]->setAlpha(config.getFloatValue("guialpha"));
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

    mBeing = 0;

    if (instances == 0)
    {
        std::for_each(background.grid, background.grid + 9, dtor<Image*>());
    }
}

void PlayerBox::draw(gcn::Graphics *graphics)
{
    if (mBeing)
    {
        // Draw character
        const int bs = getFrameSize();
        const int x = getWidth() / 2 + bs;
        const int y = getHeight() - bs;
        mBeing->drawSpriteAt(static_cast<Graphics*>(graphics), x, y);
    }

    if (config.getFloatValue("guialpha") != mAlpha)
    {
        for (int a = 0; a < 9; a++)
        {
            background.grid[a]->setAlpha(config.getFloatValue("guialpha"));
        }
    }
}

void PlayerBox::drawFrame(gcn::Graphics *graphics)
{
    int w, h, bs;
    bs = getFrameSize();
    w = getWidth() + bs * 2;
    h = getHeight() + bs * 2;

    static_cast<Graphics*>(graphics)->drawImageRect(0, 0, w, h, background);
}
