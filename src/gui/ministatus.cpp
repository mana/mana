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

#include "gui.h"
#include "progressbar.h"

#include "../localplayer.h"

MiniStatusWindow::MiniStatusWindow():
    Window("")
{
    setWindowName("MiniStatus");
    setResizable(false);
    setMovable(false);
    setTitleBarHeight(0);

    mHpBar = new ProgressBar(1.0f, 100, 20, 0, 171, 34);
    mMpBar = new ProgressBar(1.0f, 100, 20, 26, 102, 230);
    mHpLabel = new gcn::Label("");
    mMpLabel = new gcn::Label("");

    mHpBar->setPosition(0, 3);
    mMpBar->setPosition(mHpBar->getWidth() + 3, 3);

    mHpLabel->setDimension(mHpBar->getDimension());
    mMpLabel->setDimension(mMpBar->getDimension());

    mHpLabel->setForegroundColor(gcn::Color(255, 255, 255));
    mMpLabel->setForegroundColor(gcn::Color(255, 255, 255));

    mHpLabel->setFont(speechFont);
    mMpLabel->setFont(speechFont);

    mHpLabel->setAlignment(gcn::Graphics::CENTER);
    mMpLabel->setAlignment(gcn::Graphics::CENTER);

    add(mHpBar);
    add(mMpBar);
    add(mHpLabel);
    add(mMpLabel);

    setDefaultSize(0, 0, mMpBar->getX() + mMpBar->getWidth(),
                   mMpBar->getY() + mMpBar->getHeight());
}

void MiniStatusWindow::update()
{
    // HP Bar coloration
    if (player_node->mHp < int(player_node->mMaxHp / 3))
    {
        mHpBar->setColor(223, 32, 32); // Red
    }
    else if (player_node->mHp < int((player_node->mMaxHp / 3) * 2))
    {
        mHpBar->setColor(230, 171, 34); // Orange
    }
    else
    {
        mHpBar->setColor(0, 171, 34); // Green
    }

    mHpBar->setProgress((float)player_node->mHp / (float)player_node->mMaxHp);
    // mpBar->setProgress((float)player_node->mp / (float)player_node->maxMp);

    // Update and center labels
    std::stringstream updatedText;
    updatedText << player_node->mHp;
    mHpLabel->setCaption(updatedText.str());
    updatedText.str("");
    updatedText << player_node->mMp;
    mMpLabel->setCaption(updatedText.str());
}

void MiniStatusWindow::draw(gcn::Graphics *graphics)
{
    update();
    Window::drawContent(graphics);
}
