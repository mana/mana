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

#include "gui/setup.h"
#include "gui/viewport.h"

#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"

#include "engine.h"
#include "game.h"
#include "particle.h"
#include "map.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

DebugWindow::DebugWindow():
    Window(_("Debug"))
{
    setWindowName("Debug");
    setupWindow->registerWindowForReset(this);

    setResizable(true);
    setCloseButton(true);
    setSaveVisible(true);
    setDefaultSize(400, 100, ImageRect::CENTER);

    mFPSLabel = new Label(strprintf(_("%d FPS"), 0));
    mMusicFileLabel = new Label(strprintf(_("Music: %s"), ""));
    mMapLabel = new Label(strprintf(_("Map: %s"), ""));
    mMinimapLabel = new Label(strprintf(_("Minimap: %s"), ""));
    mTileMouseLabel = new Label(strprintf(_("Tile: (%d, %d)"), 0, 0));
    mParticleCountLabel = new Label(strprintf(_("Particle Count: %d"), 0));

    place(0, 0, mFPSLabel, 3);
    place(3, 0, mTileMouseLabel);
    place(0, 1, mMusicFileLabel, 3);
    place(3, 1, mParticleCountLabel);
    place(0, 2, mMapLabel, 4);
    place(0, 3, mMinimapLabel, 4);

    loadWindowState();
}

void DebugWindow::logic()
{
    if (!isVisible())
        return;

    // Get the current mouse position
    int mouseTileX = (viewport->getMouseX() + viewport->getCameraX()) / 32;
    int mouseTileY = (viewport->getMouseY() + viewport->getCameraY()) / 32;

    mFPSLabel->setCaption(strprintf(_("%d FPS"), fps));

    mTileMouseLabel->setCaption(strprintf(_("Tile: (%d, %d)"), mouseTileX,
                                          mouseTileY));

    Map *currentMap = engine->getCurrentMap();
    if (currentMap)
    {
        // TODO: Add gettext support below
        const std::string music =
            "Music: " + currentMap->getProperty("music");
        mMusicFileLabel->setCaption(music);

        const std::string minimap =
            "Minimap: " + currentMap->getProperty("minimap");
        mMinimapLabel->setCaption(minimap);

        const std::string map =
            "Map: " + currentMap->getProperty("_filename");
        mMapLabel->setCaption(map);
    }

    mParticleCountLabel->setCaption(strprintf(_("Particle count: %d"),
                                    Particle::particleCount));
}
