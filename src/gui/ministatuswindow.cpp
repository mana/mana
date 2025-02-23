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
#include "game.h"
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

#include "resources/statuseffectdb.h"
#include "resources/theme.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/time.h"

#include <algorithm>

static constexpr int ICON_SPACING = 3;

MiniStatusWindow::MiniStatusWindow():
    Popup("MiniStatus")
{
    setPadding(3);
    setMinHeight(0);

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

    updateSize();

    auto stateIt = config.windows.find(getPopupName());
    setVisible(stateIt != config.windows.end() ? stateIt->second.visible.value_or(true)
                                               : true);

    mTextPopup = new TextPopup();

    addMouseListener(this);
}

MiniStatusWindow::~MiniStatusWindow() = default;

void MiniStatusWindow::drawIcons(Graphics *graphics)
{
    const auto game = Game::instance();
    const int tileWidth = game->getCurrentTileWidth();
    const int tileHeight = game->getCurrentTileHeight();

    int iconX = mXpBar->getX() + mXpBar->getWidth() + ICON_SPACING + tileWidth / 2;
    int iconY = ICON_SPACING + tileHeight;

    for (auto &icon : mStatusIcons)
    {
        icon.sprite->draw(graphics,
                          iconX - icon.sprite->getWidth() / 2,
                          iconY - icon.sprite->getHeight());
        iconX += ICON_SPACING + icon.sprite->getWidth();
    }
}

void MiniStatusWindow::event(Event::Channel channel, const Event &event)
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
            const int id = event.getInt("index");
            const bool newStatus = event.getBool("newStatus");

            auto effect = StatusEffectDB::getStatusEffect(id);
            if (!effect)
                return;

            effect->deliverMessage(newStatus);
            effect->playSfx(newStatus);

            Sprite *sprite = newStatus ? effect->getIconSprite() : nullptr;
            auto it = std::find_if(mStatusIcons.begin(), mStatusIcons.end(),
                                   [id](const StatusIcon &icon) {
                                       return icon.effectId == id;
                                   });

            if (!sprite && it != mStatusIcons.end())
                mStatusIcons.erase(it);
            else if (sprite && it == mStatusIcons.end())
                mStatusIcons.push_back(StatusIcon{id, std::unique_ptr<Sprite>(sprite)});

            updateSize();
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

    for (auto &icon : mStatusIcons)
        icon.sprite->update(Time::deltaTimeMs());
}

void MiniStatusWindow::draw(gcn::Graphics *graphics)
{
    drawChildren(graphics);

    drawIcons(static_cast<Graphics*>(graphics));
}

void MiniStatusWindow::mouseMoved(gcn::MouseEvent &event)
{
    Popup::mouseMoved(event);

    std::string tooltip1;
    std::string tooltip2;

    if (event.getSource() == mXpBar)
    {
        const int xp = PlayerInfo::getAttribute(EXP);
        const int xpNeeded = PlayerInfo::getAttribute(EXP_NEEDED);
        tooltip1 = strprintf("%u/%u", xp, xpNeeded);
        tooltip2 = strprintf("%s: %u", _("Need"), xpNeeded - xp);
    }
    else if (event.getSource() == mHpBar)
    {
        const int hp = PlayerInfo::getAttribute(HP);
        const int maxHp = PlayerInfo::getAttribute(MAX_HP);
        tooltip1 = strprintf("%u/%u", hp, maxHp);
    }
    else if (event.getSource() == mMpBar)
    {
        const int mp = PlayerInfo::getAttribute(MP);
        const int maxMp = PlayerInfo::getAttribute(MAX_MP);
        tooltip1 = strprintf("%u/%u", mp, maxMp);
    }
    else
    {
        // Check if the mouse is over one of the status icons
        const auto game = Game::instance();
        const int tileWidth = game->getCurrentTileWidth();
        const int tileHeight = game->getCurrentTileHeight();

        int iconX = mXpBar->getX() + mXpBar->getWidth() + ICON_SPACING + tileWidth / 2;
        int iconY = ICON_SPACING + tileHeight;

        for (const auto &icon : mStatusIcons)
        {
            int spriteX = iconX + icon.sprite->getOffsetX() - icon.sprite->getWidth() / 2;
            int spriteY = iconY + icon.sprite->getOffsetY() - icon.sprite->getHeight();

            if (event.getX() >= spriteX &&
                event.getX() < spriteX + icon.sprite->getWidth() &&
                event.getY() >= spriteY &&
                event.getY() < spriteY + icon.sprite->getHeight())
            {
                auto effect = StatusEffectDB::getStatusEffect(icon.effectId);
                if (effect)
                    tooltip1 = effect->name;
                break;
            }

            iconX += ICON_SPACING + icon.sprite->getWidth();
        }
    }

    if (tooltip1.empty())
    {
        mTextPopup->setVisible(false);
    }
    else
    {
        mTextPopup->show(event.getX() + getX(),
                         event.getY() + getY(),
                         tooltip1,
                         tooltip2);
    }
}

void MiniStatusWindow::mouseExited(gcn::MouseEvent &event)
{
    Popup::mouseExited(event);

    mTextPopup->setVisible(false);
}

void MiniStatusWindow::updateSize()
{
    int width = mXpBar->getX() + mXpBar->getWidth();
    int height = mXpBar->getY() + mXpBar->getHeight();

    // Increase width based on the size of the status icons
    if (!mStatusIcons.empty())
    {
        width += ICON_SPACING;
        for (const auto &icon : mStatusIcons)
            width += ICON_SPACING + icon.sprite->getWidth();
    }

    setContentSize(width, height);
}
