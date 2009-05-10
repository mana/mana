/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
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

#include "localplayer.h"

#include "gui/widgets/avatar.h"

#include "gui/widgets/icon.h"
#include "gui/widgets/label.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <stdio.h>

namespace {
    Image *avatarStatusOffline;
    Image *avatarStatusOnline;
    int avatarCount = 0;
}

Avatar::Avatar(const std::string &name):
    mName(name)
{
    setOpaque(false);
    setSize(200, 12);
    mHpState = "???";
    mMaxHpState = "???";

    if (avatarCount == 0)
    {
        ResourceManager *resman = ResourceManager::getInstance();
        avatarStatusOffline = resman->getImage("graphics/gui/circle-gray.png");
        avatarStatusOnline = resman->getImage("graphics/gui/circle-green.png");
    }
    avatarCount++;
    avatarStatusOffline->incRef();
    avatarStatusOnline->incRef();

    mStatus = new Icon(avatarStatusOffline);
    mStatus->setSize(12, 12);
    add(mStatus, 1, 0);
    mAvatarLabel.str("");
    if (mName != player_node->getName())
        mAvatarLabel << mName << "  " << mHpState << "/" + mMaxHpState;
    else
        mAvatarLabel << mName << "  " << player_node->getHp() << "/" << player_node->getMaxHp();
    mLabel = new Label(mAvatarLabel.str());
    mLabel->setSize(174, 12);
    add(mLabel, 16, 0);
}

Avatar::~Avatar()
{
    avatarCount--;

    avatarStatusOffline->decRef();
    avatarStatusOnline->decRef();
}

void Avatar::setName(const std::string &name)
{
    mName = name;
    updateAvatarLabel();
}

void Avatar::setOnline(bool online)
{
    mStatus->setImage(online ? avatarStatusOnline : avatarStatusOffline);
}

void Avatar::setHp(int hp)
{
    if (hp)
        mHpState = strprintf("%i", hp);
    else
        mHpState = "???";
    updateAvatarLabel();
}

void Avatar::setMaxHp(int maxhp)
{
    if (maxhp)
        mMaxHpState = strprintf("%i", maxhp);
    else
        mMaxHpState = "???";
    updateAvatarLabel();
}

void Avatar::updateAvatarLabel() {
    mAvatarLabel.str("");
    if (mName != player_node->getName())
        mAvatarLabel << mName << "  " << mHpState << "/" << mMaxHpState;
    else
        mAvatarLabel << mName << "  " << player_node->getHp() << "/" << player_node->getMaxHp();
    mLabel->setCaption(mAvatarLabel.str());
}
