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
int Window::instances = 0;
ImageRect Window::border;

Window::Window(const std::string& caption, bool modal, Window *parent):
    gcn::Window(caption),
    parent(parent),
    snapSize(8),
    title(true),
    modal(modal),
    resizable(false),
    mMouseResize(false),
    minWinWidth(6),
    minWinHeight(23),
    maxWinWidth(INT_MAX),
    maxWinHeight(INT_MAX)
{
    logger->log("Window::Window(\"%s\")", caption.c_str());

    if (instances == 0)
    {
        // Load static resources
        ResourceManager *resman = ResourceManager::getInstance();
        Image *dBorders = resman->getImage("graphics/gui/vscroll_grey.png");
        border.grid[0] = dBorders->getSubImage(0, 0, 4, 4);
        border.grid[1] = dBorders->getSubImage(4, 0, 3, 4);
        border.grid[2] = dBorders->getSubImage(7, 0, 4, 4);
        border.grid[3] = dBorders->getSubImage(0, 4, 4, 10);
        border.grid[4] = resman->getImage("graphics/gui/bg_quad_dis.png");
        border.grid[5] = dBorders->getSubImage(7, 4, 4, 10);
        border.grid[6] = dBorders->getSubImage(0, 15, 4, 4);
        border.grid[7] = dBorders->getSubImage(4, 15, 3, 4);
        border.grid[8] = dBorders->getSubImage(7, 15, 4, 4);
        dBorders->decRef();
    }

    instances++;

    setBorderSize(0);
    setPadding(3);
    setTitleBarHeight(20);

    // Add chrome
    chrome = new gcn::Container();
    chrome->setOpaque(false);
    setContent(chrome);

    // Add this window to the window container
    if (windowContainer) {
        windowContainer->add(this);
    }
    else {
        throw GCN_EXCEPTION("Window::Window. no windowContainer set");
    }

    // Send GUI alpha changed for initialization
    optionChanged("guialpha");
    config.addListener("guialpha", this);

    if (modal)
    {
        requestModalFocus();
    }
}

Window::~Window()
{
    logger->log("Window::~Window(\"%s\")", getCaption().c_str());

    instances--;

    if (instances == 0)
    {
        // Clean up static resources
        delete border.grid[0];
        delete border.grid[1];
        delete border.grid[2];
        delete border.grid[3];
        border.grid[4]->decRef();
        delete border.grid[5];
        delete border.grid[6];
        delete border.grid[7];
        delete border.grid[8];
    }

    config.removeListener("guialpha", this);
    delete chrome;
}

void Window::setWindowContainer(WindowContainer *wc)
{
    windowContainer = wc;
}

void Window::draw(gcn::Graphics* graphics)
{
    int x, y;
    getAbsolutePosition(x, y);

    ((Graphics*)graphics)->drawImageRect(x, y, getWidth(), getHeight(),
                                         border);

    // Draw title
    if (title) {
        graphics->setFont(getFont());
        graphics->drawText(getCaption(), 7, 5, gcn::Graphics::LEFT);
    }

    drawContent(graphics);
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

void Window::setResizable(bool r)
{
    resizable = r;
}

bool Window::getResizable()
{
    return resizable;
}

Window *Window::getParentWindow()
{
    return parent;
}

void Window::scheduleDelete()
{
    windowContainer->scheduleDelete(this);
}

void Window::add(gcn::Widget *w)
{
    chrome->add(w);
}

void Window::add(gcn::Widget *w, int x, int y)
{
    chrome->add(w, x, y);
}

void Window::mousePress(int x, int y, int button)
{
    // Let Guichan move window to top and figure out title bar drag
    gcn::Window::mousePress(x, y, button);

    // If the mouse is not inside the content, the press must have been on the
    // border, and is a candidate for a resize.
    if (getResizable() && button == 1 &&
            !getContentDimension().isPointInRect(x, y) &&
            !(mMouseDrag && y > (int)getPadding()))
    {
        mMouseResize = true;
        mMouseXOffset = x;
        mMouseYOffset = y;

        // Determine which borders are being dragged
        mLeftBorderDrag = (x < 10);
        mTopBorderDrag = (y < 10);
        mRightBorderDrag = (x >= getWidth() - 10);
        mBottomBorderDrag = (y >= getHeight() - 10);
    }
}

void Window::mouseMotion(int x, int y)
{
    if (mMouseDrag || mMouseResize)
    {
        int dx = x - mMouseXOffset;
        int dy = y - mMouseYOffset;
        gcn::Rectangle newDim = getDimension();

        // Change the dimension according to dragging and moving
        if (mMouseResize && getResizable())
        {
            if (mLeftBorderDrag)
            {
                newDim.x += dx;
                newDim.width -= dx;
            }

            if (mTopBorderDrag)
            {
                newDim.y += dy;
                newDim.height -= dy;
            }

            if (mBottomBorderDrag)
            {
                newDim.height += dy;
            }

            if (mRightBorderDrag)
            {
                newDim.width += dx;
            }
        }
        else if (mMouseDrag && isMovable())
        {
            newDim.x += dx;
            newDim.y += dy;
        }

        // Keep guichan window inside screen
        if (newDim.x < 0)
        {
            if (mMouseResize)
            {
                newDim.width += newDim.x;
            }

            newDim.x = 0;
        }
        if (newDim.y < 0)
        {
            if (mMouseResize)
            {
                newDim.height += newDim.y;
            }

            newDim.y = 0;
        }
        if (newDim.x + newDim.width > screen->w)
        {
            if (mMouseResize)
            {
                newDim.width = screen->w - newDim.x;
            }
            else
            {
                newDim.x = screen->w - newDim.width;
            }
        }
        if (newDim.y + newDim.height > screen->h)
        {
            if (mMouseResize)
            {
                newDim.height = screen->h - newDim.y;
            }
            else
            {
                newDim.y = screen->h - newDim.height;
            }
        }

        // Keep the window at least its minimum size
        int Xcorrection = 0;
        int Ycorrection = 0;

        if (newDim.width < minWinWidth)
        {
            Xcorrection = minWinWidth - newDim.width;
        }
        else if (newDim.width > maxWinWidth)
        {
            Xcorrection = maxWinWidth - newDim.width;
        }

        if (newDim.height < minWinHeight)
        {
            Ycorrection = minWinHeight - newDim.height;
        }
        else if (newDim.height > maxWinHeight)
        {
            Ycorrection = maxWinHeight - newDim.height;
        }

        if (mLeftBorderDrag) newDim.x -= Xcorrection;
        newDim.width += Xcorrection;

        if (mTopBorderDrag) newDim.y -= Ycorrection;
        newDim.height += Ycorrection;

        // Snap window to edges
        //if (x < snapSize) x = 0;
        //if (y < snapSize) y = 0;
        //if (x + winWidth + snapSize > screen->w) x = screen->w - winWidth;
        //if (y + winHeight + snapSize > screen->h) y = screen->h - winHeight;

        // Update mouse offset when dragging bottom or right border
        if (mBottomBorderDrag)
        {
            mMouseYOffset += newDim.height - getHeight();
        }
        if (mRightBorderDrag)
        {
            mMouseXOffset += newDim.width - getWidth();
        }

        // Set the new window and content dimensions
        setDimension(newDim);

        if (mContent != NULL && mMouseResize)
        {
            mContent->setDimension(getContentDimension());
        }
    }
}

void Window::mouseRelease(int x, int y, int button)
{
    if (button == 1)
    {
        mMouseResize = false;
        mMouseDrag = false;
    }
}

void Window::optionChanged(const std::string &name)
{
    if (name == "guialpha")
    {
        float guiAlpha = config.getValue("guialpha", 0.8);

        for (int i = 0; i < 9; i++)
        {
            if (border.grid[i]->getAlpha() != guiAlpha)
            {
                border.grid[i]->setAlpha(guiAlpha);
            }
        }
    }
}
