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

#include "gui.h"
#include "window.h"
#include "windowcontainer.h"
#include "../engine.h"
#include "../net/protocol.h"
#include "../main.h"

// Guichan stuff
Gui *gui;
Graphics *guiGraphics;                 // Graphics driver
gcn::SDLInput *guiInput;               // GUI input
WindowContainer *guiTop;               // The top container

Gui::Gui(Graphics *graphics)
{
    // Set graphics
    guiGraphics = graphics;
    setGraphics(graphics);

    // Set input
    guiInput = new gcn::SDLInput();
    setInput(guiInput);

    // Set image loader
#ifndef USE_OPENGL
    imageLoader = new gcn::SDLImageLoader();
#else
    hostImageLoader = new gcn::SDLImageLoader();
    imageLoader = new gcn::OpenGLImageLoader(hostImageLoader);
#endif
    gcn::Image::setImageLoader(imageLoader);

    // Initialize top GUI widget
    guiTop = new WindowContainer();
    guiTop->setDimension(gcn::Rectangle(0, 0, screen->w, screen->h));
    guiTop->setOpaque(false);
    guiTop->addMouseListener(this);
    Window::setWindowContainer(guiTop);
    setTop(guiTop);

    // Set global font
    guiFont = new gcn::ImageFont("./data/core/graphics/gui/fixedfont.png",
            " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:@"
            "!\"$%&/=?^+*#[]{}()<>_;'.,\\|-~`"
            );
    gcn::Widget::setGlobalFont(guiFont);
}

Gui::~Gui()
{
    delete guiFont;
    delete guiTop;
    delete imageLoader;
#ifdef USE_OPENGL
    delete hostImageLoader;
#endif
    delete guiInput;
}

void Gui::draw()
{
    guiGraphics->pushClipArea(guiTop->getDimension());
    guiTop->draw(guiGraphics);
    guiGraphics->popClipArea();
}

void Gui::mousePress(int mx, int my, int button)
{
    // Mouse pressed on window container (basically, the map)
    int tilex = mx / 32 + camera_x;
    int tiley = my / 32 + camera_y;

    // Experimental mouse walk support
    if (state == GAME) {
        walk(mx, my, 0);
        player_node->setPath(tiledMap->findPath(
                    player_node->x, player_node->y,
                    tilex, tiley));
    }
}
