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
 *
 *  $Id$
 */

#include "minimap.h"
#include "../being.h"
#include "../main.h"
#include "../resources/resourcemanager.h"

Minimap::Minimap():
    Window("Map"),
    mMapImage(NULL)
{
    setContentSize(100, 100);
    setPosition(20, 20);
}

Minimap::~Minimap()
{
    if (mMapImage)
    {
        mMapImage->decRef();
    }
}

void Minimap::setMap(Map *map)
{
    if (mMapImage)
    {
        mMapImage->decRef();
    }

    if (map->hasProperty("minimap"))
    {
        ResourceManager *resman = ResourceManager::getInstance();
        mMapImage = resman->getImage(map->getProperty("minimap"));

        if (mMapImage != NULL)
        {
            mMapImage->setAlpha(0.7);
        }
    }
    else
    {
        mMapImage = NULL;
    }
}

void Minimap::draw(gcn::Graphics *graphics)
{
    Window::draw(graphics);

    int x, y;
    getAbsolutePosition(x, y);

    if (mMapImage != NULL)
    {
        dynamic_cast<Graphics*>(graphics)->drawImage(
                mMapImage, x + getPadding(), y + getTitleBarHeight());
    }

    std::list<Being*>::iterator bi;

    for (bi = beings.begin(); bi != beings.end(); bi++)
    {
        Being *being = (*bi);

        if (being == player_node)
        {
            // Player dot
            graphics->setColor(gcn::Color(209, 52, 61));
            if (useOpenGL) {
#ifdef USE_OPENGL
                dynamic_cast<gcn::OpenGLGraphics*>(graphics)->fillRectangle(gcn::Rectangle(
                        being->x / 2 + getPadding() - 1,
                        being->y / 2 + getTitleBarHeight() - 1, 3, 3));
#endif
            }
            else {
                dynamic_cast<gcn::SDLGraphics*>(graphics)->fillRectangle(gcn::Rectangle(
                        being->x / 2 + getPadding() - 1,
                        being->y / 2 + getTitleBarHeight() - 1, 3, 3));
            }
        }
        else
        {
            switch (being->getType()) {
                case Being::PLAYER:
                    graphics->setColor(gcn::Color(61, 52, 209));
                    break;

                case Being::MONSTER:
                    graphics->setColor(gcn::Color(209, 52, 61));
                    break;

                default:
                    break;
            }

            graphics->fillRectangle(gcn::Rectangle(
                        being->x / 2 + getPadding(),
                        being->y / 2 + getTitleBarHeight(), 1, 1));
        }
    }
}
