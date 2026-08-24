/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include <algorithm>
#include <guichan/mouseevent.hpp>

static int updateMapOrigin(int playerPosition, int playerMovement,
                           int viewSize, int imageSize, int &offset,
                           bool panning)
{
    if (viewSize >= imageSize)
    {
        offset = 0;
        return (viewSize - imageSize) / 2;
    }

    offset = panning ? offset + playerMovement
                     : std::clamp(offset + playerMovement,
                                  std::min(0, offset), std::max(0, offset));

    const int centeredOrigin = viewSize / 2 - playerPosition;
    const int origin = std::clamp(centeredOrigin + offset, viewSize - imageSize, 0);
    offset = origin - centeredOrigin;
    return origin;
}

Minimap::Minimap():
    Window(SkinType::Popup, _("Map"))
{
    setWindowName("Minimap");
    setResizable(true);
    setMinimumContentSize(64, 64);
    setDefaultSize(5, 25, 150, 150);
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
    mMap = map;
    mPanOffsetX = 0;
    mPanOffsetY = 0;
    mPanning = false;
    mResetView = true;

    // Set the title for the Minimap
    std::string caption;

    if (map)
        caption = map->getName();

    if (caption.empty())
        caption = _("Map");

    setCaption(caption);

    // Adapt the image
    mMapImage = nullptr;
    mWidthProportion = 0.5f;
    mHeightProportion = 0.5f;

    if (map)
    {
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
        mWidthProportion = (float) mMapImage->getWidth() / map->getWidth();
        mHeightProportion = (float) mMapImage->getHeight() / map->getHeight();

        if (config.showMinimap)
            setVisible(true);
    }
    else if (!isSticky())
    {
        setVisible(false);
    }
}

void Minimap::toggle()
{
    setVisible(!isVisible(), isSticky());
    config.showMinimap = isVisible();
}

void Minimap::mousePressed(gcn::MouseEvent &event)
{
    Window::mousePressed(event);

    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        mPanning = !event.isConsumed() && !mouseResize && !mMoved && mMapImage &&
                   getChildrenArea().isPointInRect(event.getX(), event.getY());
        if (mPanning)
        {
            mPanMouseX = event.getX();
            mPanMouseY = event.getY();
            event.consume();
        }
    }
}

void Minimap::mouseDragged(gcn::MouseEvent &event)
{
    if (!mPanning || event.getButton() != gcn::MouseEvent::LEFT)
    {
        Window::mouseDragged(event);
        return;
    }

    mPanOffsetX += event.getX() - mPanMouseX;
    mPanOffsetY += event.getY() - mPanMouseY;
    mPanMouseX = event.getX();
    mPanMouseY = event.getY();
    event.consume();
}

void Minimap::mouseReleased(gcn::MouseEvent &event)
{
    if (event.getButton() == gcn::MouseEvent::LEFT)
        mPanning = false;

    Window::mouseReleased(event);
}

void Minimap::drawContent(gcn::Graphics *graphics)
{
    if (!mMap)
        return;

    auto g = static_cast<Graphics*>(graphics);
    const gcn::Rectangle a = getChildrenArea();

    g->pushClipRect(a); // does actual clipping
    g->pushClipArea(a); // only applies an offset

    const int tileWidth = mMap->getTileWidth();
    const int tileHeight = mMap->getTileHeight();

    int mapOriginX = 0;
    int mapOriginY = 0;

    if (mMapImage)
    {
        const Vector &p = local_player->getPosition();
        const int playerX = (int) (p.x * mWidthProportion) / tileWidth;
        const int playerY = (int) (p.y * mHeightProportion) / tileHeight;

        if (mResetView)
        {
            mLastPlayerX = playerX;
            mLastPlayerY = playerY;
            mResetView = false;
        }

        // Pan along with the player when the minimap does not fit the
        // window, and center it otherwise
        mapOriginX = updateMapOrigin(playerX, playerX - mLastPlayerX,
                                     a.width, mMapImage->getWidth(),
                                     mPanOffsetX, mPanning);
        mapOriginY = updateMapOrigin(playerY, playerY - mLastPlayerY,
                                     a.height, mMapImage->getHeight(),
                                     mPanOffsetY, mPanning);
        mLastPlayerX = playerX;
        mLastPlayerY = playerY;

        g->drawImage(mMapImage, mapOriginX, mapOriginY);
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

        g->fillRectangle(
            gcn::Rectangle((int) (pos.x * mWidthProportion) / tileWidth + mapOriginX - offsetWidth,
                           (int) (pos.y * mHeightProportion) / tileHeight + mapOriginY
                               - offsetHeight,
                           dotSize,
                           dotSize));
    }

    g->popClipArea();
    g->popClipRect();
}
