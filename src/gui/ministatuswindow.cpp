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

#include "gui/ministatuswindow.h"

#include "configuration.h"
#include "graphics.h"
#include "playerinfo.h"
#include "sprite.h"
#include "statuseffect.h"

#include "gui/gui.h"
#include "gui/statuswindow.h"
#include "gui/textpopup.h"

#include "gui/widgets/progressbar.h"

#include "net/net.h"
#include "net/playerhandler.h"
#include "net/gamehandler.h"

#include "net/tmwa/protocol.h"

#include "resources/theme.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/time.h"

MiniStatusWindow::MiniStatusWindow():
    Popup("MiniStatus")
{
    setPadding(3);

    listen(Event::AttributesChannel);
    listen(Event::ActorSpriteChannel);

    mHpBar = new ProgressBar(0, 100, 20, Theme::PROG_HP);
    StatusWindow::updateHPBar(mHpBar);

    if (Net::getGameHandler()->canUseMagicBar())
    {
        mMpBar = new ProgressBar(0, 100, 20,
                        Net::getPlayerHandler()->canUseMagic()
                        ? Theme::PROG_MP : Theme::PROG_NO_MP);

        StatusWindow::updateMPBar(mMpBar);
    }
    else
        mMpBar = nullptr;

    mXpBar = new ProgressBar(0, 100, 20, Theme::PROG_EXP);
    StatusWindow::updateXPBar(mXpBar);

    // Add the progressbars to the window

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

    auto stateIt = config.windows.find(getPopupName());
    setVisible(stateIt != config.windows.end() ? stateIt->second.visible.value_or(true)
                                               : true);

    mTextPopup = new TextPopup();

    addMouseListener(this);
}

void MiniStatusWindow::setIcon(int index, Sprite *sprite)
{
    if (index >= (int) mIcons.size())
        mIcons.resize(index + 1);

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
    int icon_x = mXpBar->getX() + mXpBar->getWidth() + 14;
    for (auto &icon : mIcons)
    {
        if (icon)
        {
            icon->draw(graphics, icon_x, 3);
            icon_x += 2 + icon->getWidth();
        }
    }
}

void MiniStatusWindow::event(Event::Channel channel,
                             const Event &event)
{
    if (channel == Event::AttributesChannel)
    {
        if (event.getType() == Event::UpdateAttribute)
        {
            int id = event.getInt("id");
            if (id == HP || id == MAX_HP)
            {
                StatusWindow::updateHPBar(mHpBar);
            }
            else if (id == MP || id == MAX_MP)
            {
                StatusWindow::updateMPBar(mMpBar);
            }
            else if (id == EXP || id == EXP_NEEDED)
            {
                StatusWindow::updateXPBar(mXpBar);
            }
        }
        if (event.getType() == Event::UpdateStat)
        {
            if (Net::getNetworkType() == ServerType::TMWATHENA &&
                    event.getInt("id") == TmwAthena::MATK)
            {
                StatusWindow::updateMPBar(mMpBar);
            }
        }
    }
    else if (channel == Event::ActorSpriteChannel)
    {
        if (event.getType() == Event::UpdateStatusEffect)
        {
            int index = event.getInt("index");
            bool newStatus = event.getBool("newStatus");

            StatusEffect *effect = StatusEffect::getStatusEffect(index,
                                                                 newStatus);

            if (effect)
            {
                effect->deliverMessage();
                effect->playSFX();

                Sprite *sprite = effect->getIcon();

                if (!sprite)
                {
                    // delete sprite, if necessary
                    for (unsigned int i = 0; i < mStatusEffectIcons.size();)
                        if (mStatusEffectIcons[i] == index)
                        {
                            mStatusEffectIcons.erase(mStatusEffectIcons.begin()
                                                     + i);
                            miniStatusWindow->eraseIcon(i);
                        }
                        else
                            i++;
                }
                else
                {
                    // replace sprite or append
                    bool found = false;

                    for (unsigned int i = 0; i < mStatusEffectIcons.size();
                         i++)
                        if (mStatusEffectIcons[i] == index)
                        {
                            miniStatusWindow->setIcon(i, sprite);
                            found = true;
                            break;
                        }

                    if (!found)
                    { // add new
                        int offset = mStatusEffectIcons.size();
                        miniStatusWindow->setIcon(offset, sprite);
                        mStatusEffectIcons.push_back(index);
                    }
                }
            }
        }
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
            << (int)(((float)local_player->mXpForNextLevel - (float)local_player->mXp)
             / (float)config.getValue("xpBarMonsterCounterExp", 0))
            << " "
            << config.getValue("xpBarMonsterCounterName", "Monsters") <<" left...";
    }
    */

    for (auto &icon : mIcons)
        if (icon)
            icon->update(Time::deltaTimeMs());
}

void MiniStatusWindow::mouseMoved(gcn::MouseEvent &event)
{
    Popup::mouseMoved(event);

    const int x = event.getX();
    const int y = event.getY();

    if (event.getSource() == mXpBar)
    {
        mTextPopup->show(x + getX(), y + getY(),
                         strprintf("%u/%u", PlayerInfo::getAttribute(EXP),
                                   PlayerInfo::getAttribute(EXP_NEEDED)),
                         strprintf("%s: %u", _("Need"),
                                   PlayerInfo::getAttribute(EXP_NEEDED)
                                   - PlayerInfo::getAttribute(EXP)));
    }
    else if (event.getSource() == mHpBar)
    {
        mTextPopup->show(x + getX(), y + getY(),
                         strprintf("%u/%u", PlayerInfo::getAttribute(HP),
                                   PlayerInfo::getAttribute(MAX_HP)));
    }
    else if (event.getSource() == mMpBar)
    {
        mTextPopup->show(x + getX(), y + getY(),
                         strprintf("%u/%u", PlayerInfo::getAttribute(MP),
                                   PlayerInfo::getAttribute(MAX_MP)));
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
