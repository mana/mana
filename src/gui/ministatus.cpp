/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "gui/ministatus.h"

#include "animatedsprite.h"
#include "configuration.h"
#include "graphics.h"
#include "localplayer.h"

#include "gui/gui.h"
#include "gui/statuswindow.h"
#include "gui/textpopup.h"
#include "gui/theme.h"

#include "gui/widgets/progressbar.h"

#include "net/net.h"
#include "net/playerhandler.h"
#include "net/gamehandler.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

extern volatile int tick_time;

MiniStatusWindow::MiniStatusWindow():
    Popup("MiniStatus")
{
    int max = player_node->getMaxHp();
    mHpBar = new ProgressBar(max ? (float) player_node->getHp() / max : 0,
                             100, 20, Theme::PROG_HP);
    if (Net::getGameHandler()->canUseMagicBar())
    {
        max = player_node->getMaxMP();
        mMpBar = new ProgressBar(max ? (float) player_node->getMaxMP() / max : 0,
                             100, 20, Net::getPlayerHandler()->canUseMagic() ?
                             Theme::PROG_MP : Theme::PROG_NO_MP);
    }
    else
        mMpBar = 0;

    max = player_node->getExpNeeded();
    mXpBar = new ProgressBar(max ? (float) player_node->getExp() / max : 0,
                             100, 20, Theme::PROG_EXP);
    mHpBar->setPosition(0, 3);
    if (mMpBar)
        mMpBar->setPosition(mHpBar->getWidth() + 3, 3);
    mXpBar->setPosition(mMpBar ? mMpBar->getX() + mMpBar->getWidth() + 3 :
                                 mHpBar->getX() + mHpBar->getWidth() + 3, 3);

    add(mHpBar);
    if (mMpBar)
        add(mMpBar);
    add(mXpBar);

    setContentSize(mXpBar->getX() + mXpBar->getWidth(),
                   mXpBar->getY() + mXpBar->getHeight());

    setVisible((bool) config.getValue(getPopupName() + "Visible", true));

    mTextPopup = new TextPopup();

    addMouseListener(this);

    update(StatusWindow::HP);
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

void MiniStatusWindow::drawIcons(Graphics *graphics)
{
    // Draw icons
    int icon_x = mXpBar->getX() + mXpBar->getWidth() + 4;
    for (unsigned int i = 0; i < mIcons.size(); i++)
    {
        if (mIcons[i])
        {
            mIcons[i]->draw(graphics, icon_x, 3);
            icon_x += 2 + mIcons[i]->getWidth();
        }
    }
}

void MiniStatusWindow::update(int id)
{
    if (id == StatusWindow::HP)
    {
        StatusWindow::updateHPBar(mHpBar);
    }
    else if (id == StatusWindow::MP)
    {
        StatusWindow::updateMPBar(mMpBar);
    }
    else if (id == StatusWindow::EXP)
    {
        StatusWindow::updateXPBar(mXpBar);
    }
}

void MiniStatusWindow::logic()
{
    Popup::logic();

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

    for (unsigned int i = 0; i < mIcons.size(); i++)
        if (mIcons[i])
            mIcons[i]->update(tick_time * 10);
}

void MiniStatusWindow::mouseMoved(gcn::MouseEvent &event)
{
    Popup::mouseMoved(event);

    const int x = event.getX();
    const int y = event.getY();

    if (event.getSource() == mXpBar)
    {
        mTextPopup->show(x + getX(), y + getY(),
                         strprintf("%u/%u", player_node->getExp(),
                                   player_node->getExpNeeded()),
                         strprintf("%s: %u", _("Need"),
                                   player_node->getExpNeeded()
                                   - player_node->getExp()));
    }
    else if (event.getSource() == mHpBar)
    {
        mTextPopup->show(x + getX(), y + getY(),
                         strprintf("%u/%u", player_node->getHp(),
                                   player_node->getMaxHp()));
    }
    else if (event.getSource() == mMpBar)
    {
        mTextPopup->show(x + getX(), y + getY(),
                         strprintf("%u/%u", player_node->getMP(),
                                   player_node->getMaxMP()));
    }
    else
    {
        mTextPopup->setVisible(false);
    }
}

void MiniStatusWindow::mouseExited(gcn::MouseEvent &event)
{
    Popup::mouseExited(event);

    mTextPopup->setVisible(false);
}


