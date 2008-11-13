/*
 *  The Mana World
 *  Copyright 2008 The Mana World Development Team
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

#include "avatar.h"

#include "../icon.h"

#include "../../resources/image.h"
#include "../../resources/resourcemanager.h"

Avatar::Avatar(const std::string &name):
    mName(name)
{
    setSize(110, 12);
    mLabel = new gcn::Label(name);
    mLabel->setSize(85, 12);
    mLabel->setPosition(25, 0);
    mStatusOffline = ResourceManager::getInstance()->getImage("graphics/gui/circle-gray.png");
    mStatusOnline = ResourceManager::getInstance()->getImage("graphics/gui/circle-green.png");
    mStatus = new Icon(mStatusOffline);
    mStatus->setSize(25, 12);
    mStatus->setPosition(0, 0);
}

void Avatar::setOnline(bool status)
{
    if (status)
    {
        mStatus->setImage(mStatusOnline);
    }
    else
    {
        mStatus->setImage(mStatusOffline);
    }
}

void Avatar::draw(gcn::Graphics *g)
{
    mLabel->draw(g);
    mStatus->draw(g);
}
