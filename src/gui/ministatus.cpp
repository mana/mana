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

#include "gui/ministatus.h"

#include "gui/gui.h"
#include "gui/status.h"

#include "gui/widgets/progressbar.h"

#include "animatedsprite.h"
#include "configuration.h"
#include "graphics.h"
#include "localplayer.h"

#include "utils/stringutils.h"

MiniStatusWindow::MiniStatusWindow():
    Popup("MiniStatus")
{
    mHpBar = new ProgressBar(0.0f, 100, 20, gcn::Color(0, 171, 34));
#ifdef EATHENA_SUPPORT
    mMpBar = new ProgressBar(0.0f, 100, 20, gcn::Color(26, 102, 230));
    mXpBar = new ProgressBar(0.0f, 100, 20, gcn::Color(143, 192, 211));
#endif

    mHpBar->setPosition(0, 3);
#ifdef EATHENA_SUPPORT
    mMpBar->setPosition(mHpBar->getWidth() + 3, 3);
    mXpBar->setPosition(mMpBar->getX() + mMpBar->getWidth() + 3, 3);
#endif

    add(mHpBar);
#ifdef EATHENA_SUPPORT
    add(mMpBar);
    add(mXpBar);
#endif

#ifdef EATHENA_SUPPORT
    setContentSize(mXpBar->getX() + mXpBar->getWidth(),
                   mXpBar->getY() + mXpBar->getHeight());
#else
    setContentSize(mHpBar->getX() + mHpBar->getWidth(),
                   mHpBar->getY() + mHpBar->getHeight());
#endif

    setVisible((bool) config.getValue(getPopupName() + "Visible", true));
}

void MiniStatusWindow::setIcon(int index, AnimatedSprite *sprite)
{
    if (index >= (int) mIcons.size())
        mIcons.resize(index + 1, NULL);

    if (mIcons[index])
        delete mIcons[index];

    mIcons[index] = sprite;
}

void MiniStatusWindow::eraseIcon(int index)
{
    mIcons.erase(mIcons.begin() + index);
}

extern volatile int tick_time;

void MiniStatusWindow::update()
{
    StatusWindow::updateHPBar(mHpBar);
#ifdef EATHENA_SUPPORT
    StatusWindow::updateMPBar(mMpBar);
    StatusWindow::updateXPBar(mXpBar);

    // Displays the number of monsters to next lvl
    // (disabled for now but interesting idea)
    /*
    if (config.getValue("xpBarMonsterCounterExp", 0)!=0)
    {
        updatedText << " | "
            << (int)(((float)player_node->mXpForNextLevel - (float)player_node->mXp)
             / (float)config.getValue("xpBarMonsterCounterExp", 0))
            << " "
            << config.getValue("xpBarMonsterCounterName", "Monsters") <<" left...";
    }
    */
#endif

    for (unsigned int i = 0; i < mIcons.size(); i++)
        if (mIcons[i])
            mIcons[i]->update(tick_time * 10);
}

void MiniStatusWindow::draw(gcn::Graphics *graphics)
{
    update();
    drawChildren(graphics);
}

void MiniStatusWindow::drawIcons(Graphics *graphics)
{
    // Draw icons
#ifdef TMWSERV_SUPPORT
    int icon_x = mHpBar->getX() + mHpBar->getWidth() + 4;
#else
    int icon_x = mXpBar->getX() + mXpBar->getWidth() + 4;
#endif
    for (unsigned int i = 0; i < mIcons.size(); i++) {
        if (mIcons[i]) {
            mIcons[i]->draw(graphics, icon_x, 3);
            icon_x += 2 + mIcons[i]->getWidth();
        }
    }
}
