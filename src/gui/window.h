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

#ifndef _TMW_WINDOW_H__
#define _TMW_WINDOW_H__

#include <iostream>
#include <allegro.h>
#include <guichan.hpp>
#include "windowcontainer.h"
#include "../resources/image.h"

/**
 * A window. This window can be dragged around and has a title bar.
 *
 * \ingroup GUI
 */
class Window : public gcn::Container, public gcn::MouseListener
{
    protected:
        gcn::Container *chrome;    /**< Contained container */
        Window *parent;            /**< The parent window */
        std::string caption;       /**< Title bar caption */
        int z;                     /**< Z position of the window */
        int mousePX;               /**< Mouse down location */
        int mousePY;               /**< Mouse down location */
        int snapSize;              /**< Snap distance to window edge */
        bool mouseDown;            /**< Mouse button state */
        bool modal;                /**< Window is modal */
        gcn::Color titlebarColor;  /**< Title bar color */
        int titlebarHeight;        /**< Height of title bar */

        Image *dLeft;              /**< Left side of title bar */
        Image *dMid;               /**< Middle of title bar */
        Image *dRight;             /**< Right side of title bar */

        /** The window container windows add themselves to. */
        static WindowContainer* windowContainer;

    public:
        /**
         * Constructor. Initializes the title to the given text and hooks
         * itself into the window container.
         *
         * @param text   The initial window title, "Window" by default.
         * @param modal  Block input to other windows.
         */
        Window(const std::string& text = "Window", bool modal = false,
                Window *parent = NULL);

        /**
         * Destructor.
         */
        virtual ~Window();

        /**
         * Sets the window container to be used by new windows.
         */
        static void setWindowContainer(WindowContainer *windowContainer);

        /**
         * Draws the window.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Adds a widget to the window.
         */
        void add(gcn::Widget *w);

        /**
         * Adds a widget to the window and also specifices its position.
         */
        void add(gcn::Widget *w, int x, int y);

        /**
         * Sets the title of the window.
         */
        void setTitle(const std::string& text);

        /**
         * Sets the dimension of the window contents.
         */
        void setDimension(const gcn::Rectangle& dimension);

        /**
         * Sets the width of the window contents.
         */
        void setWidth(int width);

        /**
         * Sets the height of the window contents.
         */
        void setHeight(int height);

        /**
         * Sets the location relative to the given widget.
         */
        void setLocationRelativeTo(gcn::Widget* widget);

        /**
         * Sets the size of this window.
         */
        void setSize(int width, int height);

        /**
         * Returns the parent window.
         *
         * @return The parent window or <code>NULL</code> if there is none.
         */
        Window *getParentWindow();

        /**
         * Returns wether this window is modal. This doesn't necessarily mean
         * that is gets input as a child modal window could get it.
         */
        bool isModal();

        // Mouse handling

        void mousePress(int mx, int my, int button);
        void mouseRelease(int mx, int my, int button);
        void mouseMotion(int mx, int my);
        void mouseOut();

        void _mouseInputMessage(const gcn::MouseInput &mouseInput);
};

#endif
