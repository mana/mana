/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#include "gui/debugwindow.h"

#include "gui/viewport.h"

#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"

#include "engine.h"
#include "game.h"
#include "particle.h"
#include "map.h"

#include "utils/stringutils.h"

DebugWindow::DebugWindow():
    Window("Debug")
{
    setWindowName("Debug");

    setResizable(true);
    setCloseButton(true);
    setSaveVisible(true);
    setDefaultSize(400, 100, ImageRect::CENTER);

    mFPSLabel = new Label("0 FPS");
    mMusicFileLabel = new Label("Music: ");
    mMapLabel = new Label("Map: ");
    mMiniMapLabel = new Label("Mini-Map: ");
    mTileMouseLabel = new Label("Mouse: 0, 0");
    mParticleCountLabel = new Label("Particle count: 0");

    place(0, 0, mFPSLabel, 3);
    place(3, 0, mTileMouseLabel);
    place(0, 1, mMusicFileLabel, 3);
    place(3, 1, mParticleCountLabel);
    place(0, 2, mMapLabel, 4);
    place(0, 3, mMiniMapLabel, 4);

    loadWindowState();
}

void DebugWindow::logic()
{
    if (!isVisible())
        return;

    // Get the current mouse position
    int mouseTileX = (viewport->getMouseX() + viewport->getCameraX()) / 32;
    int mouseTileY = (viewport->getMouseY() + viewport->getCameraY()) / 32;

    mFPSLabel->setCaption(toString(fps) + " FPS");

    mTileMouseLabel->setCaption("Tile: (" + toString(mouseTileX) + ", " +
                                toString(mouseTileY) + ")");

    Map *currentMap = engine->getCurrentMap();
    if (currentMap)
    {
        const std::string music =
            "Music: " + currentMap->getProperty("music");
        mMusicFileLabel->setCaption(music);

        const std::string minimap =
            "MiniMap: " + currentMap->getProperty("minimap");
        mMiniMapLabel->setCaption(minimap);

        const std::string map =
            "Map: " + currentMap->getProperty("_filename");
        mMapLabel->setCaption(map);
    }

    mParticleCountLabel->setCaption("Particle count: " +
                                    toString(Particle::particleCount));
}
