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

#include "window.h"
#include "../resources/resourcemanager.h"
#include "../log.h"
#include "../main.h"

WindowContainer *Window::windowContainer = NULL;

Window::Window(const std::string& caption, bool modal, Window *parent):
    gcn::Window(caption),
    parent(parent),
    snapSize(8),
    modal(modal),
    resizeable(false),
    minWinWidth(256),
    minWinHeight(128),
    maxWinWidth(512),
    maxWinHeight(512)
{
    logger.log("Window::Window(\"%s\")", caption.c_str());

    setBorderSize(0);
    setPadding(3);

    // Load dialog title bar image
    ResourceManager *resman = ResourceManager::getInstance();
    dBorders = resman->getImage("graphics/gui/vscroll_grey.png");
    dBackground = resman->getImage("graphics/gui/bg_quad_dis.png");

    border.grid[0] = dBorders->getSubImage(0, 0, 4, 4);
    border.grid[1] = dBorders->getSubImage(4, 0, 3, 4);
    border.grid[2] = dBorders->getSubImage(7, 0, 4, 4);
    border.grid[3] = dBorders->getSubImage(0, 4, 4, 10);
    border.grid[4] = resman->getImage("graphics/gui/bg_quad_dis.png");
    border.grid[5] = dBorders->getSubImage(7, 4, 4, 10);
    border.grid[6] = dBorders->getSubImage(0, 15, 4, 4);
    border.grid[7] = dBorders->getSubImage(4, 15, 3, 4);
    border.grid[8] = dBorders->getSubImage(7, 15, 4, 4);

    // Add chrome
    chrome = new gcn::Container();
    chrome->setOpaque(false);
    setContent(chrome);

    // Add this window to the window container
    if (windowContainer) {
        windowContainer->add(this, modal);
    }
    else {
        throw GCN_EXCEPTION("Window::Window. no windowContainer set");
    }

    // Load GUI alpha setting
    guiAlpha = config.getValue("guialpha", 0.8f);

    // Set GUI alpha level
    dBackground->setAlpha(guiAlpha);
    dBorders->setAlpha(guiAlpha);
    config.addListener("guialpha", this);
}

Window::~Window()
{
    logger.log("Window::~Window(\"%s\")", getCaption().c_str());

    // Free dialog bitmaps
    //release_bitmap(dLeft);
    //release_bitmap(dMid);
    //release_bitmap(dRight);

    config.removeListener("guialpha", this);
    delete chrome;
}

void Window::setWindowContainer(WindowContainer *wc)
{
    windowContainer = wc;
}

void Window::logic()
{
    if (mContent != NULL)
    {
        mContent->logic();
    }
}

void Window::draw(gcn::Graphics* graphics)
{
    int x, y;
    getAbsolutePosition(x, y);

    ((Graphics*)graphics)->drawImageRect(x, y, getWidth(), getHeight(),
                                         border);

    // Draw title
    graphics->setFont(getFont());
    graphics->drawText(getCaption(), 7, 5, gcn::Graphics::LEFT);

    if (mContent != NULL)
    {
        graphics->pushClipArea(getContentDimension());
        graphics->pushClipArea(gcn::Rectangle(
                    0, 0, mContent->getWidth(), mContent->getHeight()));
        mContent->draw(graphics);
        graphics->popClipArea();
        graphics->popClipArea();
    }
}

void Window::setContentWidth(int width)
{
    chrome->setWidth(width);
    resizeToContent();
}

void Window::setContentHeight(int height)
{
    chrome->setHeight(height);
    resizeToContent();
}

void Window::setLocationRelativeTo(gcn::Widget* widget)
{
    int wx, wy;
    int x, y;

    widget->getAbsolutePosition(wx, wy);
    getAbsolutePosition(x, y);

    setPosition(
            getX() + (wx + (widget->getWidth() - getWidth()) / 2 - x),
            getY() + (wy + (widget->getHeight() - getHeight()) / 2 - y));
}

void Window::setContentSize(int width, int height)
{
    setContentWidth(width);
    setContentHeight(height);
}

void Window::setMinWidth(unsigned int width)
{
    minWinWidth = width;
}

void Window::setMinHeight(unsigned int height)
{
    minWinHeight = height;
}

void Window::setMaxWidth(unsigned int width)
{
    maxWinWidth = width;
}

void Window::setMaxHeight(unsigned int height)
{
    maxWinHeight = height;
}

void Window::setResizeable(bool r)
{
    resizeable = r;
}

bool Window::getResizeable()
{
    return resizeable;
}

Window *Window::getParentWindow()
{
    return parent;
}

bool Window::isModal()
{
    return modal;
}

void Window::add(gcn::Widget *w)
{
    chrome->add(w);
}

void Window::add(gcn::Widget *w, int x, int y)
{
    chrome->add(w, x, y);
}

void Window::mouseMotion(int mx, int my)
{
    if (mMouseDrag && isMovable())
    {
        int x = mx - mMouseXOffset + getX();
        int y = my - mMouseYOffset + getY();

        // Keep guichan window inside window
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x + getWidth() > screen->w) x = screen->w - getWidth();
        if (y + getHeight() > screen->h) y = screen->h - getHeight();

        // Snap window to edges
        //if (x < snapSize) x = 0;
        //if (y < snapSize) y = 0;
        //if (x + winWidth + snapSize > screen->w) x = screen->w - winWidth;
        //if (y + winHeight + snapSize > screen->h) y = screen->h - winHeight;

        if (resizeable && mx > getWidth() - 16) {
            // Resize
            if (mx < minWinWidth)
                mx = minWinWidth;
            if (my < minWinHeight)
                my = minWinHeight;
            if (mx >= maxWinWidth)
                mx = maxWinWidth - 1;
            if (my >= maxWinHeight)
                my = maxWinHeight - 1;

            setWidth(mx);
            setHeight(my);
        } else {
            // Move
            setPosition(x, y);
        }
    }
}

void Window::optionChanged(const std::string &name)
{
    if (name == "guialpha")
    {
        guiAlpha = config.getValue("guialpha", 0.8);

        if (dBackground->getAlpha() != guiAlpha)
        {
            dBackground->setAlpha(guiAlpha);
            dBorders->setAlpha(guiAlpha);
        }
    }
}
