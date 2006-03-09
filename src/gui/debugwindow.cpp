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

#include <guichan/widgets/label.hpp>
#include <sstream>
#include <SDL_mouse.h>

#include "button.h"

#include "../game.h"
#include "../engine.h"
#include "../map.h"

DebugWindow::DebugWindow():
    Window("Debug")
{
    setWindowName("Debug");

    setResizable(true);
    setDefaultSize(0, 0, 400, 100);
    loadWindowState();

    mFPSLabel = new gcn::Label("[0 FPS]");
    mFPSLabel->setPosition(0,0);

    mMusicFileLabel = new gcn::Label("Music File: ");
    mMusicFileLabel->setPosition(0, 20);

    mMapFileLabel = new gcn::Label("Mini-Map File: ");
    mMapFileLabel->setPosition(0, 40);

    mTileMouseLabel = new gcn::Label("[Mouse: 0, 0]");
    mTileMouseLabel->setPosition(100, 0);

    Button *closeButton = new Button("Close", "close", this);
    closeButton->setPosition(5, 60);

    add(mFPSLabel);
    add(mMusicFileLabel);
    add(mMapFileLabel);
    add(mTileMouseLabel);
    add(closeButton);
}

void
DebugWindow::logic()
{
    // Get the current mouse position
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    int mouseTileX = mouseX / 32 + camera_x;
    int mouseTileY = mouseY / 32 + camera_y;

    std::stringstream updatedText;
    updatedText << "[" << fps << " FPS]";
    mFPSLabel->setCaption(updatedText.str());
    mFPSLabel->adjustSize();

    updatedText.str("");
    updatedText << "[Mouse: " << mouseTileX << ", " << mouseTileY << "]";
    mTileMouseLabel->setCaption(updatedText.str());
    mTileMouseLabel->adjustSize();

    updatedText.str("");
    mCurrentMap = engine->getCurrentMap();

    if (mCurrentMap != NULL)
    {
        updatedText << " [Music File: "
                    << mCurrentMap->getProperty("music") << "]";
        mMusicFileLabel->setCaption(updatedText.str());
        mMusicFileLabel->adjustSize();
        updatedText.str("");
        updatedText << " [MiniMap File: "
                    << mCurrentMap->getProperty("minimap") << "]";
        mMapFileLabel->setCaption(updatedText.str());
        mMapFileLabel->adjustSize();
    }
}

void
DebugWindow::action(const std::string& eventId)
{
    if (eventId == "close")
    {
        setVisible(false);
    }
}
