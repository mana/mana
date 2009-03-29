/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "playerbox.h"

#include "../animatedsprite.h"
#include "../configuration.h"
#include "../graphics.h"
#include "../player.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

#include "../utils/dtor.h"

int PlayerBox::instances = 0;
float PlayerBox::mAlpha = 1.0;
ImageRect PlayerBox::background;

PlayerBox::PlayerBox(const Player *player):
    mPlayer(player)
{
    setFrameSize(2);

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
                background.grid[a]->setAlpha(config.getValue("guialpha", 0.8));
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
    if (mPlayer)
    {
        // Draw character
        const int bs = getFrameSize();
#ifdef TMWSERV_SUPPORT
        const int x = getWidth() / 2 + bs;
        const int y = getHeight() - bs - 8;
        mPlayer->draw(static_cast<Graphics*>(graphics), x, y);
#else
        const int x = getWidth() / 2 - 16 + bs;
        const int y = getHeight() / 2 + bs;
        for (int i = 0; i < Being::VECTOREND_SPRITE; i++)
        {
            if (mPlayer->getSprite(i))
            {
                mPlayer->getSprite(i)->draw(static_cast<Graphics*>(graphics), x, y);
            }
        }
#endif
    }

    if (config.getValue("guialpha", 0.8) != mAlpha)
    {
        for (int a = 0; a < 9; a++)
        {
            background.grid[a]->setAlpha(config.getValue("guialpha", 0.8));
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
