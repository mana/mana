/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
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

#include "debugwindow.h"

#include <SDL_mouse.h>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "gui.h"
#include "viewport.h"

#include "../game.h"
#include "../engine.h"
#include "../particle.h"
#include "../map.h"

#include "../utils/tostring.h"

DebugWindow::DebugWindow():
    Window("Debug")
{
    setResizable(true);
    setCloseButton(true);
    setDefaultSize(0, 0, 400, 100);
    loadWindowState("Debug");

    mFPSLabel = new gcn::Label("[0 FPS]");
    mFPSLabel->setPosition(0,0);

    mMusicFileLabel = new gcn::Label("Music File: ");
    mMusicFileLabel->setPosition(0, 20);

    mMapFileLabel = new gcn::Label("Mini-Map File: ");
    mMapFileLabel->setPosition(0, 40);

    mTileMouseLabel = new gcn::Label("[Mouse: 0, 0]");
    mTileMouseLabel->setPosition(100, 0);

    mParticleCountLabel = new gcn::Label("[Particle count: 0]");
    mParticleCountLabel->setPosition(100, 60);

    add(mFPSLabel);
    add(mMusicFileLabel);
    add(mMapFileLabel);
    add(mTileMouseLabel);
    add(mParticleCountLabel);
}

void
DebugWindow::logic()
{
    // Get the current mouse position
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    int mouseTileX = mouseX + viewport->getCameraX();
    int mouseTileY = mouseY + viewport->getCameraY();

    mFPSLabel->setCaption("[" + toString(fps) + " FPS");
    mFPSLabel->adjustSize();

    mTileMouseLabel->setCaption("[Mouse: " +
            toString(mouseTileX) + ", " + toString(mouseTileY) + "]");
    mTileMouseLabel->adjustSize();

    Map *currentMap = engine->getCurrentMap();
    if (currentMap != NULL)
    {
        const std::string music =
            " [Music File: " + currentMap->getProperty("music") + "]";
        mMusicFileLabel->setCaption(music);
        mMusicFileLabel->adjustSize();

        const std::string minimap =
            " [MiniMap File: " + currentMap->getProperty("minimap") + "]";
        mMapFileLabel->setCaption(minimap);
        mMapFileLabel->adjustSize();
    }

    mParticleCountLabel->setCaption("[Particle count: " +
                                    toString(Particle::particleCount)
                                    +"]");
    mParticleCountLabel->adjustSize();
}
