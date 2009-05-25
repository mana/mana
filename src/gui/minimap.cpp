/*
 *  The Mana World
 *  Copyright (C) 2004-2005  The Mana World Development Team
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

#include "gui/minimap.h"

#include "being.h"
#include "beingmanager.h"
#include "configuration.h"
#include "graphics.h"
#include "localplayer.h"
#include "log.h"
#include "map.h"
#include "player.h"

#include "gui/palette.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

#include "utils/gettext.h"

#include <guichan/font.hpp>

bool Minimap::mShow = true;

Minimap::Minimap():
    Window(_("Map")),
    mMapImage(0),
    mWidthProportion(0.5),
    mHeightProportion(0.5)
{
    setWindowName("MiniMap");
    mShow = config.getValue(getWindowName() + "Show", true);
    setDefaultSize(5, 25, 100, 100);
    // set this to false as the minimap window size is changed
    //depending on the map size
    setResizable(false);

    setDefaultVisible(true);
    setSaveVisible(true);

    setStickyButton(true);
    setSticky(false);

    loadWindowState();
    setVisible(mShow, isSticky());
}

Minimap::~Minimap()
{
    config.setValue(getWindowName() + "Show", mShow);

    if (mMapImage)
        mMapImage->decRef();
}

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
    if (mMapImage)
    {
        mMapImage->decRef();
        mMapImage = 0;
    }

    if (map)
    {
        ResourceManager *resman = ResourceManager::getInstance();
        mMapImage = resman->getImage(map->getProperty("minimap"));
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

        if (mShow)
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
    mShow = isVisible();
}

void Minimap::draw(gcn::Graphics *graphics)
{
    Window::draw(graphics);

    const gcn::Rectangle a = getChildrenArea();

    graphics->pushClipArea(a);

    int mapOriginX = 0;
    int mapOriginY = 0;

    if (mMapImage)
    {
        if (mMapImage->getWidth() > a.width ||
            mMapImage->getHeight() > a.height)
        {
            const Vector &p = player_node->getPosition();
            mapOriginX = (int) (((a.width) / 2) - (int) (p.x * mWidthProportion) / 32);
            mapOriginY = (int) (((a.height) / 2) - (int) (p.y * mHeightProportion) / 32);

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

    const Beings &beings = beingManager->getAll();

    for (Beings::const_iterator bi = beings.begin(), bi_end = beings.end();
         bi != bi_end; ++bi)
    {
        const Being *being = (*bi);
        int dotSize = 2;

        switch (being->getType())
        {
            case Being::PLAYER:
                {
                    const Player *player = static_cast<const Player*>(being);

                    Palette::ColorType type = Palette::PC;

                    if (being == player_node)
                    {
                        type = Palette::SELF;
                        dotSize = 3;
                    }
                    else if (player->isGM())
                    {
                        type = Palette::GM_NAME;
                    }
                    else if (player->isInParty())
                    {
                        type = Palette::PARTY;
                    }

                    graphics->setColor(guiPalette->getColor(type));
                    break;
                 }

            case Being::MONSTER:
                graphics->setColor(guiPalette->getColor(Palette::MONSTER));
                break;

            case Being::NPC:
                graphics->setColor(guiPalette->getColor(Palette::NPC));
                break;

            default:
                continue;
        }


        const int offsetHeight = (int) ((dotSize - 1) * mHeightProportion);
        const int offsetWidth = (int) ((dotSize - 1) * mWidthProportion);
        const Vector &pos = being->getPosition();

        graphics->fillRectangle(gcn::Rectangle(
                    (int) (pos.x * mWidthProportion) / 32 + mapOriginX - offsetWidth,
                    (int) (pos.y * mHeightProportion) / 32 + mapOriginY - offsetHeight,
                    dotSize, dotSize));
    }

    graphics->popClipArea();
}
