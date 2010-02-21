/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "gui/debugwindow.h"

#include "game.h"
#include "particle.h"
#include "main.h"
#include "map.h"

#include "gui/setup.h"
#include "gui/setup_video.h"
#include "gui/viewport.h"

#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"

#include "resources/image.h"

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

#ifdef USE_OPENGL
    if (Image::getLoadAsOpenGL())
    {
        mFPSText = _("%d FPS (OpenGL)");
    }
    else
#endif
    {
        mFPSText = _("%d FPS");
    }

    mFPSLabel = new Label(strprintf(_("%d FPS"), 0));
    mMusicFileLabel = new Label(strprintf(_("Music: %s"), ""));
    mMapLabel = new Label(strprintf(_("Map: %s"), ""));
    mMinimapLabel = new Label(strprintf(_("Minimap: %s"), ""));
    mTileMouseLabel = new Label(strprintf(_("Cursor: (%d, %d)"), 0, 0));
    mParticleCountLabel = new Label(strprintf(_("Particle count: %d"), 88888));
    mParticleDetailLabel = new Label();
    mAmbientDetailLabel = new Label();

    place(0, 0, mFPSLabel, 3);
    place(3, 0, mTileMouseLabel);
    place(0, 1, mMusicFileLabel, 3);
    place(3, 1, mParticleCountLabel);
    place(0, 2, mMapLabel, 4);
    place(3, 2, mParticleDetailLabel);
    place(0, 3, mMinimapLabel, 4);
    place(3, 3, mAmbientDetailLabel);

    loadWindowState();
}

void DebugWindow::logic()
{
    if (!isVisible())
        return;

    mFPSLabel->setCaption(strprintf(mFPSText.c_str(), fps));

    if (const Map *map = Game::instance()->getCurrentMap())
    {
          // Get the current mouse position
        int mouseTileX = (viewport->getMouseX() + viewport->getCameraX())
                        / map->getTileWidth();
        int mouseTileY = (viewport->getMouseY() + viewport->getCameraY())
                        / map->getTileHeight();
        mTileMouseLabel->setCaption(strprintf(_("Cursor: (%d, %d)"),
                        mouseTileX,
                        mouseTileY));

        mMusicFileLabel->setCaption(strprintf(
            _("Music: %s"), map->getProperty("music").c_str()));
        mMinimapLabel->setCaption(
            strprintf(_("Minimap: %s"), map->getProperty("minimap").c_str()));
        mMapLabel->setCaption(
            strprintf(_("Map: %s"), map->getProperty("_filename").c_str()));
    }

    mParticleCountLabel->setCaption(strprintf(_("Particle count: %d"),
                                    Particle::particleCount));

    mParticleCountLabel->adjustSize();

    mParticleDetailLabel->setCaption(strprintf(_("Particle detail: %s"),
                                    Setup_Video::particleDetailToString()));

    mParticleDetailLabel->adjustSize();

    mAmbientDetailLabel->setCaption(strprintf(_("Ambient FX: %s"),
                                    Setup_Video::overlayDetailToString()));

    mAmbientDetailLabel->adjustSize();
}
