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

bool Minimap::mShow = true;

Minimap::Minimap():
    Window(_("MiniMap")),
    mMapImage(NULL),
    mProportion(0.5)
{
    setWindowName("MiniMap");
    mShow = config.getValue(getWindowName() + "Show", true);
    setDefaultSize(5, 25, 100, 100);
    setResizable(true);

    loadWindowState();
}

Minimap::~Minimap()
{
    if (mMapImage)
        mMapImage->decRef();

    config.setValue(getWindowName() + "Show", mShow);
}

void Minimap::setMapImage(Image *img)
{
    if (mMapImage)
        mMapImage->decRef();

    mMapImage = img;

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
        setMaxWidth(mMapImage->getWidth() > titleWidth ?
                    mMapImage->getWidth() + offsetX : titleWidth);
        setMaxHeight(mMapImage->getHeight() + offsetY);

        setDefaultSize(getX(), getY(), getWidth(), getHeight());
        resetToDefaultSize();

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
}

void Minimap::draw(gcn::Graphics *graphics)
{
    setVisible(mShow);

    if (!isVisible())
        return;

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
#ifdef TMWSERV_SUPPORT
            const Vector &p = player_node->getPosition();
            mapOriginX = (int) (((a.width) / 2) - (int) (p.x * mProportion) / 32);
            mapOriginY = (int) (((a.height) / 2) - (int) (p.y * mProportion) / 32);
#else
            mapOriginX = (int) (((a.width) / 2) - (player_node->mX * mProportion));
            mapOriginY = (int) (((a.height) / 2) - (player_node->mY * mProportion));
#endif

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
        const Vector &pos = being->getPosition();

        graphics->fillRectangle(gcn::Rectangle(
                    (int) (pos.x * mProportion) / 32 + mapOriginX - offset,
                    (int) (pos.x * mProportion) / 32 + mapOriginY - offset,
                    dotSize, dotSize));
    }

    graphics->popClipArea();
}
