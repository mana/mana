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
#include "focushandler.h"
#include "../engine.h"
#include "../net/protocol.h"
#include "../main.h"
#include "../game.h"

// Guichan stuff
Gui *gui;
Graphics *guiGraphics;                 // Graphics driver
gcn::SDLInput *guiInput;               // GUI input
WindowContainer *guiTop;               // The top container

// Fonts used in showing hits
gcn::ImageFont *hitRedFont;
gcn::ImageFont *hitBlueFont;
gcn::ImageFont *hitYellowFont;

Gui::Gui(Graphics *graphics)
{
    // Set graphics
    guiGraphics = graphics;
    setGraphics(graphics);

    // Set input
    guiInput = new gcn::SDLInput();
    setInput(guiInput);

    // Set image loader
#ifdef USE_OPENGL
    if (useOpenGL) {
        hostImageLoader = new gcn::SDLImageLoader();
        imageLoader = new gcn::OpenGLImageLoader(hostImageLoader);
    }
    else {
        hostImageLoader = NULL;
        imageLoader = new gcn::SDLImageLoader();
    }
#else
    imageLoader = new gcn::SDLImageLoader();
#endif

    gcn::Image::setImageLoader(imageLoader);

    // Set focus handler
    delete mFocusHandler;
    mFocusHandler = new FocusHandler();

    // Initialize top GUI widget
    guiTop = new WindowContainer();
    guiTop->setDimension(gcn::Rectangle(0, 0, screen->w, screen->h));
    guiTop->setOpaque(false);
    guiTop->addMouseListener(this);
    Window::setWindowContainer(guiTop);
    setTop(guiTop);

    // Set global font
    try {
        guiFont = new gcn::ImageFont(
                TMW_DATADIR "data/graphics/gui/fixedfont.png",
                " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567"
                "89:@!\"$%&/=?^+*#[]{}()<>_;'.,\\|-~`"
                );
    }
    catch (gcn::Exception e)
    {
        logger->error("Unable to load fixedfont.png!");
    }

    // Set hits' colourful fonts
    try {
        hitRedFont = new gcn::ImageFont(
                TMW_DATADIR "data/graphics/gui/hits_red.png",
                "0123456789");
        hitBlueFont = new gcn::ImageFont(
                TMW_DATADIR "data/graphics/gui/hits_blue.png",
                "0123456789");
        hitYellowFont = new gcn::ImageFont(
                TMW_DATADIR "data/graphics/gui/hits_yellow.png",
                "mis");
    }
    catch (gcn::Exception e)
    {
        logger->error("Unable to load colored hits' fonts!");
    }

    gcn::Widget::setGlobalFont(guiFont);
}

Gui::~Gui()
{
    // Fonts used in showing hits
    delete hitRedFont;
    delete hitBlueFont;
    delete hitYellowFont;
    
    delete guiFont;
    delete guiTop;
    delete imageLoader;
#ifdef USE_OPENGL
    if (hostImageLoader) {
        delete hostImageLoader;
    }
#endif
    delete guiInput;
}

void Gui::logic()
{
    gcn::Gui::logic();

    // Work around Guichan bug of only applying focus on mouse or keyboard
    // events.
    mFocusHandler->applyChanges();
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

    // When conditions for walking are met, set new player destination
    if (player_node && player_node->action != DEAD && current_npc == 0 &&
            button == gcn::MouseInput::LEFT)
    {
        Map *tiledMap = engine->getCurrentMap();
        int tilex = mx / 32 + camera_x;
        int tiley = my / 32 + camera_y;

        if (state == GAME && tiledMap->getWalk(tilex, tiley)) {
            walk(tilex, tiley, 0);
            player_node->setDestination(tilex, tiley);
        }

        autoTarget = NULL;
    }
}


gcn::ImageFont *Gui::getFont()
{
    return guiFont;
}

