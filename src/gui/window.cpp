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

#include <guichan/exception.hpp>

#include "gccontainer.h"
#include "windowcontainer.h"

#include "../configlistener.h"
#include "../configuration.h"
#include "../graphics.h"
#include "../log.h"

#include "../graphic/imagerect.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

ConfigListener *Window::windowConfigListener = NULL;
WindowContainer *Window::windowContainer = NULL;
int Window::instances = 0;
ImageRect Window::border;
Image *Window::resizeGrip;

class WindowConfigListener : public ConfigListener
{
    public:
        /**
         * Called when an config option changes.
         */
        void optionChanged(const std::string &name)
        {
            if (name == "guialpha")
            {
                float guiAlpha = config.getValue("guialpha", 0.8);

                for (int i = 0; i < 9; i++)
                {
                    if (Window::border.grid[i]->getAlpha() != guiAlpha)
                    {
                        Window::border.grid[i]->setAlpha(guiAlpha);
                    }
                }
            }
        }
};

Window::Window(const std::string& caption, bool modal, Window *parent):
    gcn::Window(caption),
    mParent(parent),
    mWindowName("window"),
    mSnapSize(8),
    mShowTitle(true),
    mModal(modal),
    mResizable(false),
    mMouseResize(false),
    mMinWinWidth(6),
    mMinWinHeight(23),
    mMaxWinWidth(INT_MAX),
    mMaxWinHeight(INT_MAX)
{
    logger->log("Window::Window(\"%s\")", caption.c_str());

    if (!windowContainer) {
        throw GCN_EXCEPTION("Window::Window. no windowContainer set");
    }

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
        resizeGrip = resman->getImage("graphics/gui/resize.png");
        dBorders->decRef();
        windowConfigListener = new WindowConfigListener();
        // Send GUI alpha changed for initialization
        windowConfigListener->optionChanged("guialpha");
        config.addListener("guialpha", windowConfigListener);
    }

    instances++;

    setBorderSize(0);
    setPadding(3);
    setTitleBarHeight(20);

    // Add chrome
    mChrome = new GCContainer();
    mChrome->setOpaque(false);
    setContent(mChrome);

    // Add this window to the window container
    windowContainer->add(this);

    if (mModal)
    {
        requestModalFocus();
    }
}

Window::~Window()
{
    logger->log("Window::~Window(\"%s\")", getCaption().c_str());
    const std::string &name = mWindowName;

    // Saving X, Y and Width and Height for resizables in the config
    config.setValue(name + "WinX", getX());
    config.setValue(name + "WinY", getY());

    if (mResizable)
    {
        config.setValue(name + "WinWidth", getWidth());
        config.setValue(name + "WinHeight", getHeight());
    }

    instances--;

    if (instances == 0)
    {
        config.removeListener("guialpha", windowConfigListener);
        delete windowConfigListener;
        windowConfigListener = NULL;

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
        resizeGrip->decRef();
    }

    delete mChrome;
}

void Window::setWindowContainer(WindowContainer *wc)
{
    windowContainer = wc;
}

void Window::draw(gcn::Graphics* graphics)
{
    Graphics *g = (Graphics*)graphics;

    g->drawImageRect(0, 0, getWidth(), getHeight(), border);

    // Draw grip
    if (mResizable)
    {
        g->drawImage(Window::resizeGrip,
                     getWidth() - resizeGrip->getWidth(),
                     getHeight() - resizeGrip->getHeight());
    }

    // Draw title
    if (mShowTitle) {
        graphics->setFont(getFont());
        graphics->drawText(getCaption(), 7, 5, gcn::Graphics::LEFT);
    }

    drawContent(graphics);
}

void Window::setContentWidth(int width)
{
    mChrome->setWidth(width);
    resizeToContent();
}

void Window::setContentHeight(int height)
{
    mChrome->setHeight(height);
    resizeToContent();
}

void Window::setLocationRelativeTo(gcn::Widget* widget)
{
    int wx, wy;
    int x, y;

    widget->getAbsolutePosition(wx, wy);
    getAbsolutePosition(x, y);

    setPosition(getX() + (wx + (widget->getWidth() - getWidth()) / 2 - x),
                getY() + (wy + (widget->getHeight() - getHeight()) / 2 - y));
}

void Window::setContentSize(int width, int height)
{
    setContentWidth(width);
    setContentHeight(height);
}

void Window::setMinWidth(unsigned int width)
{
    mMinWinWidth = width;
}

void Window::setMinHeight(unsigned int height)
{
    mMinWinHeight = height;
}

void Window::setMaxWidth(unsigned int width)
{
    mMaxWinWidth = width;
}

void Window::setMaxHeight(unsigned int height)
{
    mMaxWinHeight = height;
}

void Window::setResizable(bool r)
{
    mResizable = r;
}

bool Window::isResizable()
{
    return mResizable;
}

void Window::scheduleDelete()
{
    windowContainer->scheduleDelete(this);
}

void Window::add(gcn::Widget *w, bool delChild)
{
    mChrome->add(w, delChild);
}

void Window::add(gcn::Widget *w, int x, int y, bool delChild)
{
    mChrome->add(w, x, y, delChild);
}

void Window::mousePress(int x, int y, int button)
{
    // Let Guichan move window to top and figure out title bar drag
    gcn::Window::mousePress(x, y, button);

    // If the mouse is not inside the content, the press must have been on the
    // border, and is a candidate for a resize.
    if (isResizable() && button == 1 &&
        getGripDimension().isPointInRect(x, y) &&
        !getContentDimension().isPointInRect(x, y) &&
        hasMouse() &&
        !(mMouseDrag && y > (int)getPadding()))
    {
        mMouseResize = true;
        mMouseXOffset = x;
        mMouseYOffset = y;
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
        if (mMouseResize && isResizable())
        {
            // We're dragging bottom right
            newDim.height += dy;
            newDim.width += dx;
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
        if (newDim.x + newDim.width > windowContainer->getWidth())
        {
            if (mMouseResize)
            {
                newDim.width = windowContainer->getWidth() - newDim.x;
            }
            else
            {
                newDim.x = windowContainer->getWidth() - newDim.width;
            }
        }
        if (newDim.y + newDim.height > windowContainer->getHeight())
        {
            if (mMouseResize)
            {
                newDim.height = windowContainer->getHeight() - newDim.y;
            }
            else
            {
                newDim.y = windowContainer->getHeight() - newDim.height;
            }
        }

        // Keep the window at least its minimum size
        int Xcorrection = 0;
        int Ycorrection = 0;

        if (newDim.width < mMinWinWidth)
        {
            Xcorrection = mMinWinWidth - newDim.width;
        }
        else if (newDim.width > mMaxWinWidth)
        {
            Xcorrection = mMaxWinWidth - newDim.width;
        }

        if (newDim.height < mMinWinHeight)
        {
            Ycorrection = mMinWinHeight - newDim.height;
        }
        else if (newDim.height > mMaxWinHeight)
        {
            Ycorrection = mMaxWinHeight - newDim.height;
        }

        // Snap window to edges
        //if (x < snapSize) x = 0;
        //if (y < snapSize) y = 0;
        //if (x + winWidth + snapSize > screen->w) x = screen->w - winWidth;
        //if (y + winHeight + snapSize > screen->h) y = screen->h - winHeight;

        // Update mouse offset when dragging bottom or right border
        if (mMouseResize)
        {
            mMouseYOffset += newDim.height - getHeight();
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

void
Window::mouseRelease(int x, int y, int button)
{
    if (button == 1)
    {
        mMouseResize = false;
        mMouseDrag = false;
    }
}

gcn::Rectangle
Window::getGripDimension()
{
    return gcn::Rectangle(getWidth() - resizeGrip->getWidth(),
                          getHeight() - resizeGrip->getHeight(),
                          getWidth(),
                          getHeight());
}

void
Window::loadWindowState()
{
    const std::string &name = mWindowName;

    setPosition((int)config.getValue(name + "WinX", getX()),
                (int)config.getValue(name + "WinY", getY()));

    if (mResizable)
    {
        setWidth((int)config.getValue(name + "WinWidth", getWidth()));
        setHeight((int)config.getValue(name + "WinHeight", getHeight()));

        if (mContent != NULL)
        {
            mContent->setDimension(getContentDimension());
        }
    }
}

void Window::setDefaultSize(int defaultX, int defaultY,
                            int defaultWidth, int defaultHeight)
{
    mDefaultX = defaultX;
    mDefaultY = defaultY;
    mDefaultWidth = defaultWidth;
    mDefaultHeight = defaultHeight;

    setPosition(mDefaultX, mDefaultY);
    setContentSize(mDefaultWidth, mDefaultHeight);
}

void Window::resetToDefaultSize()
{
    setPosition(mDefaultX, mDefaultY);
    setContentSize(mDefaultWidth, mDefaultHeight);

    if (mContent != NULL)
    {
        mContent->setDimension(getContentDimension());
    }
}
