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
 */

#include "ministatus.h"

#include <guichan/widgets/label.hpp>

#include "gui.h"
#include "progressbar.h"

#include "../localplayer.h"
#include "../configuration.h"
#include "../graphics.h"

#include "../utils/tostring.h"

MiniStatusWindow::MiniStatusWindow()
{
    setResizable(false);
    setMovable(false);
    setTitleBarHeight(0);

    mHpBar = new ProgressBar(1.0f, 100, 20, 0, 171, 34);
    mHpLabel = new gcn::Label("");

    mHpBar->setPosition(0, 3);

    mHpLabel->setDimension(mHpBar->getDimension());

    mHpLabel->setForegroundColor(gcn::Color(255, 255, 255));

    mHpLabel->setFont(speechFont);

    mHpLabel->setAlignment(gcn::Graphics::CENTER);

    add(mHpBar);
    add(mHpLabel);
}

void MiniStatusWindow::update()
{
    // HP Bar coloration
    int maxHp = player_node->getMaxHP();
    int hp = player_node->getHP();
    if (hp < int(maxHp / 3))
    {
        mHpBar->setColor(223, 32, 32); // Red
    }
    else if (hp < int((maxHp / 3) * 2))
    {
        mHpBar->setColor(230, 171, 34); // Orange
    }
    else
    {
        mHpBar->setColor(0, 171, 34); // Green
    }

    mHpBar->setProgress((float) hp / maxHp);

    // Update labels
    mHpLabel->setCaption(toString(hp));
}

void MiniStatusWindow::draw(gcn::Graphics *graphics)
{
    update();
    drawChildren(graphics);
}
