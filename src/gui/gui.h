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

#ifndef _TMW_GUI
#define _TMW_GUI

#include <guichan.hpp>
#include <guichan/sdl.hpp>
#include <string.h>
#include "windowcontainer.h"
#include "../graphic/graphic.h"

/**
 * \defgroup GUI GUI related classes
 */

/**
 * Main GUI class.
 *
 * \ingroup GUI
 */
class Gui
{
    public:
        /**
         * Constructor.
         */
        Gui(Graphics *screen);

        /**
         * Destructor.
         */
        ~Gui();

        /**
         * Focus none of the Widgets in the Gui.
         */
        void focusNone();

        /**
         * Performs the Gui:s logic by calling all logic functions
         * down in the Gui heirarchy. Logic can be just about anything
         * like adjusting a Widgets size or doing some calculations.
         *
         * NOTE: Logic also deals with user input (Mouse and Keyboard)
         *       for Widgets.
         */
        void logic();

        /**
         * Draws the whole Gui by calling draw functions down in the
         * Gui hierarchy. It also draws the mouse pointer.
         */
        void draw();

    private:
        gcn::Gui *gui;                        /**< The GUI system */
#ifdef USE_OPENGL
        gcn::ImageLoader *hostImageLoader;    /**< For loading images in GL */
#endif
        gcn::ImageLoader *imageLoader;        /**< For loading images */
        gcn::ImageFont *guiFont;              /**< The global GUI font */

        bool topHasMouse;

        gcn::FocusHandler *focusHandler;
};

extern Gui *gui;
extern WindowContainer *guiTop;               // The top container
extern Graphics *guiGraphics;                 // Graphics driver
extern gcn::SDLInput *guiInput;               // GUI input

/** Initialize gui system */
void init_gui(Graphics *graphics);

void gui_exit();

#endif
