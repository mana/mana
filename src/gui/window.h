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
#include <guichan.hpp>
#include "windowcontainer.h"
#include "../resources/image.h"
#include "../graphics.h"

/**
 * A window. This window can be dragged around and has a title bar.
 *
 * \ingroup GUI
 */
class Window : public gcn::Window
{
    protected:
        gcn::Container *chrome;    /**< Contained container */
        Window *parent;            /**< The parent window */
        int snapSize;              /**< Snap distance to window edge */
        bool modal;                /**< Window is modal */
        float guiAlpha;            /**< Opacity of GUI windows */

        Image *dBackground;        /**< Left side of title bar */
        Image *dBorders;           /**< Middle of title bar */

        ImageRect border;          /**< The window border */

        /** The window container windows add themselves to. */
        static WindowContainer* windowContainer;

    public:
        /**
         * Constructor. Initializes the title to the given text and hooks
         * itself into the window container.
         *
         * @param text   The initial window title, "Window" by default.
         * @param modal  Block input to other windows.
         * @param parent The parent window. This is the window standing above
         *               this one in the window hiearchy. When reordering,
         *               a window will never go below its parent window.
         */
        Window(const std::string &caption = "Window", bool modal = false,
                Window *parent = NULL);

        /**
         * Destructor.
         */
        ~Window();

        /**
         * Sets the window container to be used by new windows.
         */
        static void setWindowContainer(WindowContainer *windowContainer);

        /**
         * Draws the window.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Calls logic on content widget (cause Guichan 0.3.0 forgot this)
         */
        void logic();

        /**
         * Adds a widget to the window.
         */
        void add(gcn::Widget *w);

        /**
         * Adds a widget to the window and also specifices its position.
         */
        void add(gcn::Widget *w, int x, int y);

        /**
         * Sets the width of the window contents.
         */
        void setContentWidth(int width);

        /**
         * Sets the height of the window contents.
         */
        void setContentHeight(int height);

        /**
         * Sets the location relative to the given widget.
         */
        void setLocationRelativeTo(gcn::Widget* widget);

        /**
         * Sets the size of this window.
         */
        void setContentSize(int width, int height);

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

        /**
         * Window dragging. This method also makes sure the window is not
         * dragged outside of the screen.
         */
        void mouseMotion(int mx, int my);
};

#endif
