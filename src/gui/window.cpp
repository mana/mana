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
#include "gui.h"
#include <guichan/allegro.hpp>

Window::Window(gcn::Container *parent, const std::string& text) :
    caption(text),
    mousePX(0),
    mousePY(0),
    snapSize(8),
    mouseDown(false),
    titlebarHeight(20)
{
    titlebarColor.r = 203;
    titlebarColor.g = 203;
    titlebarColor.b = 203;

    setBaseColor(gcn::Color(255, 255, 255));

    // Load dialog title bar image
    dLeft = load_bitmap("data/Skin/dialogLeft.bmp", NULL);
    dMid = load_bitmap("data/Skin/dialogMiddle.bmp", NULL);
    dRight = load_bitmap("data/Skin/dialogRight.bmp", NULL);

    // Register mouse listener
    addMouseListener(this);

    // Add chrome
    chrome = new gcn::Container();
    chrome->setOpaque(false);
    chrome->setY(titlebarHeight);
    gcn::Container::add(chrome);

    // Add this window to the parent container
    parent->add(this);
}

Window::~Window()
{
    // Free dialog bitmaps
    release_bitmap(dLeft);
    release_bitmap(dMid);
    release_bitmap(dRight);

    delete chrome;
}

void Window::draw(gcn::Graphics* graphics)
{
    // Draw container background when opaque
    if (mOpaque) {
        graphics->setColor(getBaseColor());
        graphics->fillRectangle(gcn::Rectangle(0, titlebarHeight,
                    getDimension().width,
                    getDimension().height - titlebarHeight));
    }

    // Draw line around window
    graphics->setColor(titlebarColor);
    graphics->drawRectangle(gcn::Rectangle(0, titlebarHeight - 1,
                getWidth(), getHeight() - titlebarHeight + 1));

    // Skinned dialog render
    if (typeid(*graphics) == typeid(gcn::AllegroGraphics))
    {
        gcn::AllegroGraphics *gfx = (gcn::AllegroGraphics*)graphics;
        BITMAP *screen = gfx->getTarget();
        int x, y, i;
        getAbsolutePosition(x, y);

        // Draw title bar
        masked_blit(dLeft, screen, 0, 0, x, y, 24, 24);
        for (i = 24; i < getWidth() - 24; i += 24)
        {
            blit(dMid, screen, 0, 0, x + i, y, 24, 24);
        }
        masked_blit(dRight, screen, 0, 0,
                x + getWidth() - 24, y, 24, 24);
    }
    else {
        // Plain title bar
        graphics->setColor(titlebarColor);
        graphics->fillRectangle(gcn::Rectangle(0, 0,
                    getWidth(), titlebarHeight));
    }

    // Draw title
    graphics->setFont(getFont());
    graphics->drawText(caption, 4, 4, gcn::Graphics::LEFT);

    drawChildren(graphics);
}

void Window::setTitle(const std::string& text)
{
    caption = std::string(text);
}

void Window::setDimension(const gcn::Rectangle &dimension)
{
    gcn::Container::setDimension(gcn::Rectangle(
                dimension.x,
                dimension.y - titlebarHeight,
                dimension.width,
                dimension.height + titlebarHeight));
    chrome->setDimension(gcn::Rectangle(0, titlebarHeight,
                dimension.width, dimension.height));
}

void Window::setWidth(int width)
{
    gcn::Container::setWidth(width);
    chrome->setWidth(width);
}

void Window::setHeight(int height)
{
    gcn::Container::setHeight(height + titlebarHeight);
    chrome->setHeight(height);
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

void Window::setSize(int width, int height)
{
    setWidth(width);
    setHeight(height);
}

void Window::add(gcn::Widget *w)
{
    chrome->add(w);
}

void Window::add(Widget *w, int x, int y)
{
    chrome->add(w, x, y);
}

void Window::mousePress(int mx, int my, int button)
{
    int x = this->getDimension().x;
    int y = this->getDimension().y;

    mouseDown = true;

    mousePX = mx;
    mousePY = my;
}

void Window::mouseRelease(int mx, int my, int button)
{
    mouseDown = false;
}

void Window::mouseMotion(int mx, int my)
{
    if (mouseDown)
    {
        int winWidth = this->getDimension().width;
        int winHeight = this->getDimension().height;
        int x = this->getDimension().x;
        int y = this->getDimension().y;

        x = x - (mousePX - mx);
        y = y - (mousePY - my);

        // Keep guichan window inside window
        if (x < 0)
            x = 0;
        if (y < 0)
            y = 0;
        if (x + winWidth > 799)
            x = 799 - winWidth;
        if (y + winHeight > 599)
            y = 599 - winHeight;

        // Snap window to edges
        if (x < snapSize)
            x = 0;
        if (y < snapSize)
            y = 0;
        if (x + winWidth + snapSize > 799)
            x = 799 - winWidth;
        if (y + winHeight + snapSize > 599)
            y = 599 - winHeight;

        this->setPosition(x, y);
    }
}

void Window::mouseOut()
{
    mouseDown = false;
}
