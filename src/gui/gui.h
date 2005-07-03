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
#include "windowcontainer.h"
#include "../graphics.h"

/**
 * \defgroup GUI Core GUI related classes (widgets)
 */

/**
 * \defgroup Interface User interface related classes (windows, dialogs)
 */

/**
 * Main GUI class.
 *
 * \ingroup GUI
 */
class Gui : public gcn::Gui, public gcn::MouseListener
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
         * Works around Guichan bug
         */
        void logic();

        /**
         * Draws the whole Gui by calling draw functions down in the
         * Gui hierarchy. It also draws the mouse pointer.
         */
        void draw();

        /**
         * Handles mouse press on map.
         */
        void mousePress(int mx, int my, int button);

        /**
         * Return game font
         */
        gcn::ImageFont *getFont();

    private:
#ifdef USE_OPENGL
        gcn::ImageLoader *hostImageLoader;    /**< For loading images in GL */
#endif
        gcn::ImageLoader *imageLoader;        /**< For loading images */
        gcn::ImageFont *guiFont;              /**< The global GUI font */
};

extern Gui *gui;                              /**< The GUI system */
extern WindowContainer *guiTop;               /**< The top container */
extern Graphics *guiGraphics;                 /**< Graphics driver */
extern gcn::SDLInput *guiInput;               /**< GUI input */

/**
 * Fonts used in showing hits
 */
extern gcn::ImageFont *hitRedFont;
extern gcn::ImageFont *hitBlueFont;
extern gcn::ImageFont *hitYellowFont;

#endif
