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

#include <guichan/font.hpp>

#include "minimap.h"

#include "../being.h"
#include "../beingmanager.h"
#include "../configuration.h"
#include "../graphics.h"
#include "../localplayer.h"

#include "../resources/image.h"

#include "../utils/gettext.h"

bool Minimap::mShow = config.getValue("MinimapVisible", true);

Minimap::Minimap():
    Window(_("Map")),
    mMapImage(NULL),
    mProportion(0.5)
{
    setWindowName(_("MiniMap"));
}

Minimap::~Minimap()
{
    if (mMapImage)
        mMapImage->decRef();
}

void Minimap::setMapImage(Image *img)
{
    if (mMapImage)
        mMapImage->decRef();

    mMapImage = img;

    if (mMapImage)
    {
        const int offsetX = getPadding() + 4;
        const int offsetY = getTitleBarHeight() + 4;
        const int titleWidth = getFont()->getWidth(getCaption()) + 15;
        const int mapWidth = mMapImage->getWidth() < 100 ? 
                             mMapImage->getWidth() + offsetX : 100;
        mMapImage->setAlpha(config.getValue("guialpha", 0.8));
        setDefaultSize(offsetX, offsetY, 
                       mapWidth > titleWidth ? mapWidth : titleWidth, 
                       mMapImage->getHeight() < 100 ? 
                           mMapImage->getHeight() + offsetY : 100);
        loadWindowState();
        setVisible(mShow);
    }
    else
    {
        setVisible(false);
    }
}

void Minimap::toggle()
{
    mShow = !mShow;
    config.setValue("MinimapVisible", mShow);
}

void Minimap::draw(gcn::Graphics *graphics)
{
    setVisible(mShow);

    Window::draw(graphics);

    if (!mShow)
        return;

    const gcn::Rectangle a = getChildrenArea();

    graphics->pushClipArea(a);

    int mapOriginX = 0;
    int mapOriginY = 0;

    if (mMapImage)
    {
        if (mMapImage->getWidth() > a.width ||
            mMapImage->getHeight() > a.height)
        {
            mapOriginX = (int) (((a.width) / 2) - (player_node->mX * mProportion));
            mapOriginY = (int) (((a.height) / 2) - (player_node->mY * mProportion));

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
    Beings::const_iterator bi;

    for (bi = beings.begin(); bi != beings.end(); bi++)
    {
        const Being *being = (*bi);
        int dotSize = 2;

        switch (being->getType()) {
            case Being::PLAYER:
                if (being == player_node)
                {
                    dotSize = 3;
                    graphics->setColor(gcn::Color(61, 209, 52));
                    break;
                }
                graphics->setColor(gcn::Color(61, 52, 209));
                break;

            case Being::MONSTER:
                graphics->setColor(gcn::Color(209, 52, 61));
                break;

            case Being::NPC:
                graphics->setColor(gcn::Color(255, 255, 0));
                break;

            default:
                continue;
        }

        const int offset = (int) ((dotSize - 1) * mProportion);

        graphics->fillRectangle(gcn::Rectangle(
                    (int) (being->mX * mProportion) + mapOriginX - offset,
                    (int) (being->mY * mProportion) + mapOriginY - offset,
                    dotSize, dotSize));
    }

    graphics->popClipArea();
}
