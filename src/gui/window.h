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
 */

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <iostream>
#include <allegro.h>
#include <guichan.hpp>

/**
 * A window. This window can be dragged around and has a title bar.
 *
 * \ingroup GUI
 */
class Window : public gcn::Container, public gcn::MouseListener
{
    private:
        gcn::Container *chrome;    /**< Contained container */
        std::string caption;       /**< Title bar caption */
        int z;                     /**< Z position of the window */
        int mousePX;               /**< Mouse down location */
        int mousePY;               /**< Mouse down location */
        int snapSize;              /**< Snap distance to window edge */
        bool mouseDown;            /**< Mouse button state */
        gcn::Color titlebarColor;  /**< Title bar color */
        int titlebarHeight;        /**< Height of title bar */

        BITMAP *dLeft;             /**< Left side of title bar */
        BITMAP *dMid;              /**< Middle of title bar */
        BITMAP *dRight;            /**< Right side of title bar */

    public:
        Window(gcn::Container *parent, const std::string& text = "Window");
        ~Window();

        /**
         * Draws the window.
         */
        void draw(gcn::Graphics* graphics);

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

        // Mouse handling

        void mousePress(int mx, int my, int button);
        void mouseRelease(int mx, int my, int button);
        void mouseMotion(int mx, int my);
        void mouseOut();
};

#endif
