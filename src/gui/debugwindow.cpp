/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

#include <SDL_mouse.h>

#include <guichan/widgets/label.hpp>

#include "debugwindow.h"
#include "viewport.h"

#include "widgets/layout.h"

#include "../engine.h"
#include "../game.h"
#include "../particle.h"
#include "../map.h"

#include "../utils/stringutils.h"

DebugWindow::DebugWindow():
    Window("Debug")
{
    setWindowName("Debug");

    setResizable(true);
    setCloseButton(true);
    setDefaultSize(0, 0, 400, 60);
    loadWindowState();

    mFPSLabel = new gcn::Label("0 FPS");
    mMusicFileLabel = new gcn::Label("Music: ");
    mMapLabel = new gcn::Label("Map: ");
    mMiniMapLabel = new gcn::Label("Mini-Map: ");
    mTileMouseLabel = new gcn::Label("Mouse: 0, 0");
    mParticleCountLabel = new gcn::Label("Particle count: 0");

    place(0, 0, mFPSLabel);
    place(3, 0, mTileMouseLabel);
    place(0, 1, mMusicFileLabel, 2);
    place(3, 1, mParticleCountLabel);
    place(0, 2, mMapLabel, 2);
    place(0, 3, mMiniMapLabel, 2);

    reflowLayout(375, 0);
}

void DebugWindow::logic()
{
    // Get the current mouse position
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    int mouseTileX = (mouseX + viewport->getCameraX()) / 32;
    int mouseTileY = (mouseY + viewport->getCameraY()) / 32;

    mFPSLabel->setCaption(toString(fps) + " FPS");
    mFPSLabel->adjustSize();

    mTileMouseLabel->setCaption("Mouse: " +
            toString(mouseTileX) + ", " + toString(mouseTileY));
    mTileMouseLabel->adjustSize();

    Map *currentMap = engine->getCurrentMap();
    if (currentMap)
    {
        const std::string music =
            "Music: " + currentMap->getProperty("music");
        mMusicFileLabel->setCaption(music);
        mMusicFileLabel->adjustSize();

        const std::string minimap =
            "MiniMap: " + currentMap->getProperty("minimap");
        mMiniMapLabel->setCaption(minimap);
        mMiniMapLabel->adjustSize();

        const std::string map =
            "Map: " + currentMap->getProperty("_filename");
        mMapLabel->setCaption(map);
        mMapLabel->adjustSize();
    }

    mParticleCountLabel->setCaption("Particle count: " +
                                    toString(Particle::particleCount));
    mParticleCountLabel->adjustSize();
}
