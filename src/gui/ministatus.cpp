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

#include "ministatus.h"

// #include <guichan/widgets/label.hpp>
#include <sstream>

#include "progressbar.h"

#include "../playerinfo.h"

MiniStatusWindow::MiniStatusWindow():
    Window("")
{
    setResizable(false);
    setTitleBarHeight(0);
    setMovable(false);

    hpBar = new ProgressBar(1.0f,
            0, 3, 80, 15,
            0, 171, 34);

    mpBar = new ProgressBar(1.0f,
            hpBar->getX() + hpBar->getWidth() + 3,
            hpBar->getY(), 80, 15, 26, 102, 230);

    add(hpBar);
    add(mpBar);

    setContentSize(mpBar->getX() + mpBar->getWidth(),
                    mpBar->getY() + mpBar->getHeight());
}

MiniStatusWindow::~MiniStatusWindow()
{
    delete hpBar;
    delete mpBar;
}

void MiniStatusWindow::update()
{
    // HP Bar coloration
    if (player_info->hp < int(player_info->max_hp / 3))
    {
        hpBar->setColor(223, 32, 32); // Red
    }
    else
    {
        if (player_info->hp < int((player_info->max_hp / 3) * 2))
        {
            hpBar->setColor(230, 171, 34); // Orange
        }
        else
        {
            hpBar->setColor(0, 171, 34); // Green
        }
    }

    hpBar->setProgress((float)player_info->hp / (float)player_info->max_hp);
    // mpBar->setProgress((float)player_info->mp / (float)player_info->max_mp);
}

void MiniStatusWindow::draw(gcn::Graphics *graphics)
{
    update();

    Window::drawContent(graphics);
}
