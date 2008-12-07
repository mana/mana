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

MiniStatusWindow::MiniStatusWindow():
    Window("")
{
    setWindowName("MiniStatus");
    setResizable(false);
    setMovable(false);
    setTitleBarHeight(0);

    mHpBar = new ProgressBar(1.0f, 100, 20, 0, 171, 34);
    mMpBar = new ProgressBar(1.0f, 100, 20, 26, 102, 230);
    mXpBar = new ProgressBar(1.0f, 100, 20, 143, 192, 211);
    mHpLabel = new gcn::Label("");
    mMpLabel = new gcn::Label("");
    mXpLabel = new gcn::Label("");

    mHpBar->setPosition(0, 3);
    mMpBar->setPosition(mHpBar->getWidth() + 3, 3);
    mXpBar->setPosition(mMpBar->getX() + mMpBar->getWidth() + 3, 3);

    mHpLabel->setDimension(mHpBar->getDimension());
    mMpLabel->setDimension(mMpBar->getDimension());
    mXpLabel->setDimension(mXpBar->getDimension());

    mHpLabel->setForegroundColor(gcn::Color(255, 255, 255));
    mMpLabel->setForegroundColor(gcn::Color(255, 255, 255));
    mXpLabel->setForegroundColor(gcn::Color(255, 255, 255));

    mHpLabel->setFont(speechFont);
    mMpLabel->setFont(speechFont);
    mXpLabel->setFont(speechFont);

    mHpLabel->setAlignment(gcn::Graphics::CENTER);
    mMpLabel->setAlignment(gcn::Graphics::CENTER);
    mXpLabel->setAlignment(gcn::Graphics::CENTER);

    add(mHpBar);
    add(mMpBar);
    add(mXpBar);
    add(mHpLabel);
    add(mMpLabel);
    add(mXpLabel);

    setDefaultSize(0, 0, mXpBar->getX() + mXpBar->getWidth(),
                   mXpBar->getY() + mXpBar->getHeight());
    loadWindowState();
}

void
MiniStatusWindow::setIcon(int index, AnimatedSprite *sprite)
{
    if (index >= (int) mIcons.size())
        mIcons.resize(index + 1, NULL);

    if (mIcons[index])
        delete mIcons[index];

    mIcons[index] = sprite;
}

void
MiniStatusWindow::eraseIcon(int index)
{
    mIcons.erase(mIcons.begin() + index);
}

extern volatile int tick_time;

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

    mHpBar->setProgress((float) player_node->mHp / player_node->mMaxHp);
    mMpBar->setProgress((float) player_node->mMp / player_node->mMaxMp);
    if (player_node->MATK <= 0)
        mMpBar->setColor(100, 100, 100); // grey, to indicate that we lack magic
    else
        mMpBar->setColor(26, 102, 230); // blue, to indicate that we have magic

    mXpBar->setProgress(
            (float) player_node->getXp() / player_node->mXpForNextLevel);

    // Update labels
    mHpLabel->setCaption(toString(player_node->mHp));
    mMpLabel->setCaption(toString(player_node->mMp));

    std::stringstream updatedText;
    updatedText << (int) (
            (float) player_node->getXp() /
            player_node->mXpForNextLevel * 100) << "%";

    // Displays the number of monsters to next lvl
    // (disabled for now but interesting idea)
    /*
    if(config.getValue("xpBarMonsterCounterExp", 0)!=0)
    {
        updatedText << " | "
            << (int)(((float)player_node->mXpForNextLevel - (float)player_node->mXp)
             / (float)config.getValue("xpBarMonsterCounterExp", 0))
            << " "
            << config.getValue("xpBarMonsterCounterName", "Monsters") <<" left...";
    }
    */

    mXpLabel->setCaption(updatedText.str());

    for (unsigned int i = 0; i < mIcons.size(); i++)
        if (mIcons[i])
            mIcons[i]->update(tick_time * 10);

}

void MiniStatusWindow::draw(gcn::Graphics *gcn_graphics)
{
    update();
    drawChildren(gcn_graphics);
}

void
MiniStatusWindow::drawIcons(Graphics *graphics)
{
    // Draw icons
    int icon_x = mXpBar->getX() + mXpBar->getWidth() + 4;
    for (unsigned int i = 0; i < mIcons.size(); i++)
        if (mIcons[i]) {
            mIcons[i]->draw(graphics, icon_x, 3);
            icon_x += 2 + mIcons[i]->getWidth();
        }
}
