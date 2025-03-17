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
#include "map.h"

#include "gui/setup.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"
#include "resources/userpalette.h"

#include "utils/filesystem.h"
#include "utils/gettext.h"

#include <guichan/font.hpp>

Minimap::Minimap():
    Window(_("Map"))
{
    setWindowName("Minimap");
    setDefaultSize(5, 25, 100, 100);
    // set this to false as the minimap window size is changed
    //depending on the map size
    setResizable(false);
    setupWindow->registerWindowForReset(this);

    setDefaultVisible(true);
    setSaveVisible(true);

    setStickyButton(true);
    setSticky(false);

    loadWindowState();
    setVisible(config.showMinimap, isSticky());
}

Minimap::~Minimap() = default;

void Minimap::setMap(Map *map)
{
    // Set the title for the Minimap
    std::string caption;

    if (map)
        caption = map->getName();

    if (caption.empty())
        caption = _("Map");

    minimap->setCaption(caption);

    // Adapt the image
    mMapImage = nullptr;

    if (map)
    {
        mMap = map;
        std::string tempname =
            "graphics/minimaps/" + map->getFilename() + ".png";
        ResourceManager *resman = ResourceManager::getInstance();

        std::string minimapName = map->getProperty("minimap");

        if (minimapName.empty() && FS::exists(tempname))
            minimapName = tempname;

        if (!minimapName.empty())
            mMapImage = resman->getImage(minimapName);
    }

    if (mMapImage)
    {
        const int offsetX = 2 * getPadding();
        const int offsetY = getTitleBarHeight() + getPadding();
        const int titleWidth = getFont()->getWidth(getCaption()) + 15;
        const int mapWidth = mMapImage->getWidth() < 100 ?
                             mMapImage->getWidth() + offsetX : 100;
        const int mapHeight = mMapImage->getHeight() < 100 ?
                              mMapImage->getHeight() + offsetY : 100;

        setMinWidth(mapWidth > titleWidth ? mapWidth : titleWidth);
        setMinHeight(mapHeight);

        mWidthProportion = (float) mMapImage->getWidth() / map->getWidth();
        mHeightProportion = (float) mMapImage->getHeight() / map->getHeight();

        setMaxWidth(mMapImage->getWidth() > titleWidth ?
                    mMapImage->getWidth() + offsetX : titleWidth);
        setMaxHeight(mMapImage->getHeight() + offsetY);

        setDefaultSize(getX(), getY(), getWidth(), getHeight());
        resetToDefaultSize();

        if (config.showMinimap)
            setVisible(true);
    }
    else
    {
        if (!isSticky())
            setVisible(false);
    }
}

void Minimap::toggle()
{
    setVisible(!isVisible(), isSticky());
    config.showMinimap = isVisible();
}

void Minimap::draw(gcn::Graphics *graphics)
{
    Window::draw(graphics);

    const gcn::Rectangle a = getChildrenArea();

    graphics->pushClipArea(a);

    int mapOriginX = 0;
    int mapOriginY = 0;

    if (mMapImage && mMap)
    {
        if (mMapImage->getWidth() > a.width ||
            mMapImage->getHeight() > a.height)
        {
            const Vector &p = local_player->getPosition();
            mapOriginX = (int) (((a.width) / 2) - (int) (p.x * mWidthProportion)
                         / mMap->getTileWidth());
            mapOriginY = (int) (((a.height) / 2)
                         - (int) (p.y * mHeightProportion)
                         / mMap->getTileHeight());

            const int minOriginX = a.width - mMapImage->getWidth();
            const int minOriginY = a.height - mMapImage->getHeight();

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

    for (auto actor : actorSpriteManager->getAll())
    {
        if (actor->getType() == ActorSprite::FLOOR_ITEM)
            continue;

        const Being *being = static_cast<Being*>(actor);

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
