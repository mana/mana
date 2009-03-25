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
    ResourceManager *resman = ResourceManager::getInstance();
    mStatusOffline = resman->getImage("graphics/gui/circle-gray.png");
    mStatusOnline = resman->getImage("graphics/gui/circle-green.png");
    mStatus = new Icon(mStatusOffline);
    mStatus->setSize(25, 12);
    mStatus->setPosition(0, 0);
}

void Avatar::setOnline(bool online)
{
    mStatus->setImage(online ? mStatusOnline : mStatusOffline);
}

void Avatar::draw(gcn::Graphics *g)
{
    mLabel->draw(g);
    mStatus->draw(g);
}
