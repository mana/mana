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
#include "button.h"
#include "minimap.h"

#include <guichan/widgets/label.hpp>
#include <sstream>

#include "../game.h"
#include "../graphics.h"
#include "../engine.h"
#include "../map.h"

extern Graphics *graphics;
extern Minimap *minimap;
extern int frame;

DebugWindow::DebugWindow():
    Window("Debug")
{
    setWindowName("Debug");

    setResizable(true);
    setDefaultSize(0, 0, 400, 100);
    loadWindowState();

    FPSLabel = new gcn::Label("[0 FPS]");
    FPSLabel->setPosition(0,0);

    musicFileLabel = new gcn::Label("Music File: ");
    musicFileLabel->setPosition(0, 20);

    mapFileLabel = new gcn::Label("Mini-Map File: ");
    mapFileLabel->setPosition(0, 40);

    tileMouseLabel = new gcn::Label("[Mouse: 0, 0]");
    tileMouseLabel->setPosition(100, 0);

    Button *closeButton = new Button("Close");
    closeButton->setEventId("close");
    closeButton->setPosition(5, 60);
    closeButton->addActionListener(this);

    add(FPSLabel);
    add(musicFileLabel);
    add(mapFileLabel);
    add(tileMouseLabel);
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
    FPSLabel->setCaption(updatedText.str());
    FPSLabel->adjustSize();

    updatedText.str("");
    updatedText << "[Mouse: " << mouseTileX << ", " << mouseTileY << "]";
    tileMouseLabel->setCaption(updatedText.str());
    tileMouseLabel->adjustSize();

    updatedText.str("");
    mCurrentMap = engine->getCurrentMap();

    if (mCurrentMap != NULL)
    {
        updatedText << " [Music File: "
                    << mCurrentMap->getProperty("music") << "]";
        musicFileLabel->setCaption(updatedText.str());
        musicFileLabel->adjustSize();
        updatedText.str("");
        updatedText << " [MiniMap File: "
                    << mCurrentMap->getProperty("minimap") << "]";
        mapFileLabel->setCaption(updatedText.str());
        mapFileLabel->adjustSize();
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
