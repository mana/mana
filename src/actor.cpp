/*
 *  The Mana Client
 *  Copyright (C) 2010  The Mana Developers
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

#include "actor.h"

#include "map.h"

#include "resources/image.h"

Actor::Actor():
        mMap(NULL)
{}

Actor::~Actor()
{
    setMap(NULL);
}

void Actor::setMap(Map *map)
{
    // Remove Actor from potential previous map
    if (mMap)
        mMap->removeActor(mMapActor);

    mMap = map;

    // Add Actor to potential new map
    if (mMap)
        mMapActor = mMap->addActor(this);
}

int Actor::getTileX() const
{
    return getPixelX() / mMap->getTileWidth();
}

int Actor::getTileY() const
{
    return getPixelY() / mMap->getTileHeight();
}
