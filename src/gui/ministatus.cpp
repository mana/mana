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

#include <guichan/widgets/label.hpp>
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
            0, 3, 100, 20,
            0, 171, 34);

    mpBar = new ProgressBar(1.0f,
            hpBar->getX() + hpBar->getWidth() + 3,
            hpBar->getY(), 100, 20, 26, 102, 230);

    hpLabel = new gcn::Label("");
    mpLabel = new gcn::Label("");

    add(hpBar);
    add(mpBar);
    add(hpLabel);
    add(mpLabel);

    setContentSize(mpBar->getX() + mpBar->getWidth(),
                    mpBar->getY() + mpBar->getHeight());
}

MiniStatusWindow::~MiniStatusWindow()
{
    delete hpBar;
    delete mpBar;
    delete hpLabel;
    delete mpLabel;
}

void MiniStatusWindow::update()
{
    // HP Bar coloration
    if (player_info->hp < int(player_info->maxHp / 3))
    {
        hpBar->setColor(223, 32, 32); // Red
    }
    else
    {
        if (player_info->hp < int((player_info->maxHp / 3) * 2))
        {
            hpBar->setColor(230, 171, 34); // Orange
        }
        else
        {
            hpBar->setColor(0, 171, 34); // Green
        }
    }

    hpBar->setProgress((float)player_info->hp / (float)player_info->maxHp);
    // mpBar->setProgress((float)player_info->mp / (float)player_info->maxMp);

    // Update and center labels
    std::stringstream updatedText;
    updatedText << player_info->hp << "/" << player_info->maxHp;
    hpLabel->setCaption(updatedText.str());
    hpLabel->adjustSize();
    updatedText.str("");
    updatedText << player_info->mp << "/" << player_info->maxMp;
    mpLabel->setCaption(updatedText.str());
    mpLabel->adjustSize();
    hpLabel->setPosition(hpBar->getX() + int((hpBar->getWidth() / 2) - (hpLabel->getWidth() / 2)),
                        hpBar->getY() + int((hpBar->getHeight() / 2) - (hpLabel->getHeight() / 2)));
    mpLabel->setPosition(mpBar->getX() + int((mpBar->getWidth() / 2) - (mpLabel->getWidth() / 2)),
                        mpBar->getY() + int((mpBar->getHeight() / 2) - (mpLabel->getHeight() / 2)));
}

void MiniStatusWindow::draw(gcn::Graphics *graphics)
{
    update();

    Window::drawContent(graphics);
}
