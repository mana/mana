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

#include "gui/widgets/avatar.h"

#include "localplayer.h"

#include "gui/gui.h"
#include "gui/widgets/icon.h"
#include "gui/widgets/label.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

#include <sstream>

namespace {
    Image *avatarStatusOffline;
    Image *avatarStatusOnline;
    int avatarCount = 0;
}

Avatar::Avatar():
    mHp(0),
    mMaxHp(0),
    mDisplayBold(false)
{
    setOpaque(false);

    if (avatarCount == 0)
    {
        ResourceManager *resman = ResourceManager::getInstance();
        avatarStatusOffline = resman->getImage("graphics/gui/circle-gray.png");
        avatarStatusOnline = resman->getImage("graphics/gui/circle-green.png");
    }
    avatarCount++;
    avatarStatusOffline->incRef();
    avatarStatusOnline->incRef();

    mLabel = new Label;
    mLabel->adjustSize();

    mStatus = new Icon(avatarStatusOffline);
    mStatus->setSize(10, 10);

    add(mStatus, 4, (mLabel->getHeight() - 10) / 2);
    add(mLabel, 18, 0);

    setSize(250, mLabel->getHeight());

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
    if (hp == mHp)
        return;

    mHp = hp;
    updateAvatarLabel();
}

void Avatar::setMaxHp(int maxHp)
{
    if (maxHp == mMaxHp)
        return;

    mMaxHp = maxHp;
    updateAvatarLabel();
}

void Avatar::updateAvatarLabel()
{
    std::ostringstream ss;
    ss << mName;

    if (mName != player_node->getName() && mMaxHp != 0)
        ss << "  (" << mHp << "/" << mMaxHp << ")";

    if (mDisplayBold)
        mLabel->setFont(boldFont);
    mLabel->setCaption(ss.str());
    mLabel->adjustSize();
}
