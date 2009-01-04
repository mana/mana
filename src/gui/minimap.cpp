/*
 *  The Mana World
 *  Copyright 2004-2005 The Mana World Development Team
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
 */

#include "minimap.h"

#include "../being.h"
#include "../beingmanager.h"
#include "../graphics.h"
#include "../localplayer.h"

#include "../resources/image.h"

#include "../utils/gettext.h"

Minimap::Minimap():
    Window(_("MiniMap")),
    mMapImage(NULL)
{
    setWindowName("MiniMap");
    setDefaultSize(5, 25, 100, 100);
    loadWindowState();
    // LEEOR: The Window class needs to modified to accept
    // setAlignment calls.
    setAlignment(gcn::Graphics::CENTER);
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

    if (mMapImage) {
        mMapImage->setAlpha(0.7);
        setContentSize(mMapImage->getWidth(), mMapImage->getHeight());
    }
}

void Minimap::draw(gcn::Graphics *graphics)
{
    Window::draw(graphics);

    const gcn::Rectangle a = getChildrenArea();

    int mapOriginX = a.x;
    int mapOriginY = a.y;

    if (mMapImage)
    {
        if (mMapImage->getWidth() > a.width ||
            mMapImage->getHeight() > a.height)
        {
            const Vector &pos = player_node->getPosition();
            mapOriginX += (a.width - (int) (pos.x / 32)) / 2;
            mapOriginY += (a.height - (int) (pos.y / 32)) / 2;
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

            case Being::NPC:
                graphics->setColor(gcn::Color(255, 255, 0));
                break;

            case Being::MONSTER:
                graphics->setColor(gcn::Color(209, 52, 61));
                break;

            default:
                continue;
        }

        const int offset = (dotSize - 1) / 2;
        const Vector &pos = being->getPosition();

        graphics->fillRectangle(gcn::Rectangle(
                    (int) pos.x / 64 + mapOriginX - offset,
                    (int) pos.x / 64 + mapOriginY - offset,
                    dotSize, dotSize));
    }
}
