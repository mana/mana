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

#include "gui/minimap.h"

#include "actorspritemanager.h"
#include "being.h"
#include "configuration.h"
#include "graphics.h"
#include "localplayer.h"
#include "log.h"
#include "map.h"

#include "gui/setup.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"
#include "resources/userpalette.h"

#include "utils/gettext.h"

#include <guichan/font.hpp>

Minimap::Minimap():
    mMap(0),
    mMapImage(0),
    mWidthProportion(0.5),
    mHeightProportion(0.5)
{
    setSize(100, 100);
}

Minimap::~Minimap()
{
    if (mMapImage)
        mMapImage->decRef();
}

void Minimap::setMap(Map *map)
{
    // Adapt the image
    if (mMapImage)
    {
        mMapImage->decRef();
        mMapImage = 0;
    }

    if (map)
    {
        mMap = map;
        std::string tempname =
            "graphics/minimaps/" + map->getFilename() + ".png";
        ResourceManager *resman = ResourceManager::getInstance();

        std::string minimapName = map->getProperty("minimap");

        if (minimapName.empty() && resman->exists(tempname))
            minimapName = tempname;

        if (!minimapName.empty())
            mMapImage = resman->getImage(minimapName);
    }

    if (mMapImage)
    {
        mWidthProportion = (float) mMapImage->getWidth() / map->getWidth();
        mHeightProportion = (float) mMapImage->getHeight() / map->getHeight();

        setVisible(true);
    }
    else
    {
        setVisible(true);
    }
}

void Minimap::draw(gcn::Graphics *graphics)
{
    const int width = getWidth();
    const int height = getHeight();
    graphics->pushClipArea(gcn::Rectangle(0, 0, width, height));

    int mapOriginX = 0;
    int mapOriginY = 0;

    if (mMapImage && mMap)
    {

        if (mMapImage->getWidth() > width ||
            mMapImage->getHeight() > height)
        {
            const Vector &p = local_player->getPosition();
            mapOriginX = (int) ((width / 2) - (int) (p.x * mWidthProportion)
                         / mMap->getTileWidth());
            mapOriginY = (int) ((height / 2) - (int) (p.y * mHeightProportion)
                         / mMap->getTileHeight());

            const int minOriginX = width - mMapImage->getWidth();
            const int minOriginY = height - mMapImage->getHeight();

            if (mapOriginX < minOriginX)
                mapOriginX = minOriginX;
            if (mapOriginY < minOriginY)
                mapOriginY = minOriginY;
            if (mapOriginX > 0)
                mapOriginX = 0;
            if (mapOriginY > 0)
                mapOriginY = 0;
        }

        static_cast<Graphics*>(graphics)->
            drawImage(mMapImage, mapOriginX, mapOriginY);
    }

    const ActorSprites &actors = actorSpriteManager->getAll();

    for (ActorSpritesConstIterator it = actors.begin(), it_end = actors.end();
         it != it_end; it++)
    {
        if ((*it)->getType() == ActorSprite::FLOOR_ITEM)
            continue;

        const Being *being = static_cast<Being*>(*it);

        if (!being->isAlive())
            continue;

        int dotSize = 2;

        int type = UserPalette::PC;

        if (being == local_player)
        {
            type = UserPalette::SELF;
            dotSize = 3;
        }
        else if (being->isGM())
            type = UserPalette::GM;
        else if (being->isInParty())
            type = UserPalette::PARTY;
        else
        {
            switch (being->getType())
            {
                case ActorSprite::MONSTER:
                    type = UserPalette::MONSTER;
                    break;

                case ActorSprite::NPC:
                    type = UserPalette::NPC;
                    break;

                default:
                    continue;
            }
        }

        graphics->setColor(userPalette->getColor(type));

        const int offsetHeight = (int) ((dotSize - 1) * mHeightProportion);
        const int offsetWidth = (int) ((dotSize - 1) * mWidthProportion);
        const Vector &pos = being->getPosition();

        if (mMap)
        {
            graphics->fillRectangle(gcn::Rectangle(
                (int) (pos.x * mWidthProportion) / mMap->getTileWidth()
                + mapOriginX - offsetWidth,
                (int) (pos.y * mHeightProportion) / mMap->getTileHeight()
                + mapOriginY - offsetHeight,
                dotSize, dotSize));
        }
    }

    graphics->popClipArea();
}
