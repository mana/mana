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

#include "gui/widgets/icon.h"
#include "gui/widgets/label.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

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

    mLabel = new Label(name);
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
    mLabel->setCaption(name);
}

void Avatar::setOnline(bool online)
{
    mStatus->setImage(online ? avatarStatusOnline : avatarStatusOffline);
}
