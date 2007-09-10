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
#include <guichan/widgets/icon.hpp>

#include "gccontainer.h"
#include "windowcontainer.h"

#include "widgets/resizegrip.h"

#include "../configlistener.h"
#include "../configuration.h"
#include "../graphics.h"
#include "../log.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

ConfigListener *Window::windowConfigListener = 0;
WindowContainer *Window::windowContainer = 0;
int Window::instances = 0;
ImageRect Window::border;

class WindowConfigListener : public ConfigListener
{
    void optionChanged(const std::string &)
    {
        for_each(Window::border.grid, Window::border.grid + 9,
                std::bind2nd(std::mem_fun(&Image::setAlpha),
                    config.getValue("guialpha", 0.8)));
    }
};

Window::Window(const std::string& caption, bool modal, Window *parent):
    gcn::Window(caption),
    mGrip(0),
    mParent(parent),
    mModal(modal),
    mResizable(false),
    mMouseResize(0),
    mSticky(false),
    mMinWinWidth(100),
    mMinWinHeight(40),
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
    gcn::Window::add(mChrome);

    // Add this window to the window container
    windowContainer->add(this);

    if (mModal)
    {
        requestModalFocus();
    }

    // Windows are invisible by default
    setVisible(false);
}

Window::~Window()
{
    logger->log("Window::~Window(\"%s\")", getCaption().c_str());

    std::string const &name = mConfigName;
    if (!name.empty())
    {
        // Saving X, Y and Width and Height for resizables in the config
        config.setValue(name + "WinX", getX());
        config.setValue(name + "WinY", getY());

        if (mResizable)
        {
            config.setValue(name + "WinWidth", getWidth());
            config.setValue(name + "WinHeight", getHeight());
        }
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
    }

    delete mChrome;
    delete mGrip;
}

void Window::setWindowContainer(WindowContainer *wc)
{
    windowContainer = wc;
}

void Window::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics*>(graphics);

    g->drawImageRect(0, 0, getWidth(), getHeight(), border);

    // Draw title
    if (getTitleBarHeight())
    {
        graphics->setColor(gcn::Color(0, 0, 0));
        graphics->setFont(getFont());
        graphics->drawText(getCaption(), 7, 5, gcn::Graphics::LEFT);
    }

    drawChildren(graphics);
}

void Window::setContentWidth(int width)
{
    mChrome->setWidth(width);
    setWidth(width + 2 * getPadding());
}

void Window::setContentHeight(int height)
{
    mChrome->setHeight(height);
    setHeight(height + getPadding() + getTitleBarHeight());
}

void Window::setContentSize(int width, int height)
{
    setContentWidth(width);
    setContentHeight(height);
}

void Window::setWidth(int width)
{
    gcn::Window::setWidth(width);

    if (mGrip)
    {
        mGrip->setX(getWidth() - mGrip->getWidth() - getChildrenArea().x);
    }
}

void Window::setHeight(int height)
{
    gcn::Window::setHeight(height);

    if (mGrip)
    {
        mGrip->setY(getHeight() - mGrip->getHeight() - getChildrenArea().y);
    }
}

void Window::setDimension(const gcn::Rectangle &dimension)
{
    gcn::Window::setDimension(dimension);

    if (mGrip)
    {
        mGrip->setX(getWidth() - mGrip->getWidth() - getChildrenArea().x);
        mGrip->setY(getHeight() - mGrip->getHeight() - getChildrenArea().y);
    }
}

void Window::setLocationRelativeTo(gcn::Widget *widget)
{
    int wx, wy;
    int x, y;

    widget->getAbsolutePosition(wx, wy);
    getAbsolutePosition(x, y);

    setPosition(getX() + (wx + (widget->getWidth() - getWidth()) / 2 - x),
                getY() + (wy + (widget->getHeight() - getHeight()) / 2 - y));
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

    if (mResizable)
    {
        mGrip = new ResizeGrip();
        mGrip->setX(getWidth() - mGrip->getWidth() - getChildrenArea().x);
        mGrip->setY(getHeight() - mGrip->getHeight() - getChildrenArea().y);
        gcn::Window::add(mGrip);
    }
    else
    {
        delete mGrip;
        mGrip = 0;
    }
}

bool Window::isResizable()
{
    return mResizable;
}

void Window::setSticky(bool sticky)
{
    mSticky = sticky;
}

bool Window::isSticky()
{
    return mSticky;
}

void Window::setVisible(bool visible)
{
    if (isSticky())
    {
        gcn::Window::setVisible(true);
    }
    else
    {
        gcn::Window::setVisible(visible);
    }
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

void Window::mousePressed(gcn::MouseEvent &event)
{
    // Let Guichan move window to top and figure out title bar drag
    gcn::Window::mousePressed(event);

    const int x = event.getX();
    const int y = event.getY();
    mMouseResize = 0;

    // Activate resizing handles as appropriate
    if (event.getSource() == this && isResizable() &&
            event.getButton() == gcn::MouseEvent::LEFT &&
            !getChildrenArea().isPointInRect(x, y))
    {
        mMouseResize |= (x > getWidth() - resizeBorderWidth) ? RIGHT :
                        (x < resizeBorderWidth) ? LEFT : 0;
        mMouseResize |= (y > getHeight() - resizeBorderWidth) ? BOTTOM :
                        (y < resizeBorderWidth) ? TOP : 0;
    }
    else if (event.getSource() == mGrip &&
            event.getButton() == gcn::MouseEvent::LEFT)
    {
        mDragOffsetX = x;
        mDragOffsetY = y;
        mMouseResize |= BOTTOM | RIGHT;
        mIsMoving = false;
    }
}

void Window::mouseDragged(gcn::MouseEvent &event)
{
    // Let Guichan handle title bar drag
    gcn::Window::mouseDragged(event);

    // Keep guichan window inside screen when it may be moved
    if (isMovable() && mIsMoving)
    {
        int newX = std::max(0, getX());
        int newY = std::max(0, getY());
        newX = std::min(windowContainer->getWidth() - getWidth(), newX);
        newY = std::min(windowContainer->getHeight() - getHeight(), newY);
        setPosition(newX, newY);
    }

    if (mMouseResize && !mIsMoving)
    {
        const int dx = event.getX() - mDragOffsetX;
        const int dy = event.getY() - mDragOffsetY;
        gcn::Rectangle newDim = getDimension();

        if (mMouseResize & (TOP | BOTTOM))
        {
            int newHeight = newDim.height + ((mMouseResize & TOP) ? -dy : dy);
            newDim.height = std::min(mMaxWinHeight,
                                     std::max(mMinWinHeight, newHeight));

            if (mMouseResize & TOP)
            {
                newDim.y -= newDim.height - getHeight();
            }
        }

        if (mMouseResize & (LEFT | RIGHT))
        {
            int newWidth = newDim.width + ((mMouseResize & LEFT) ? -dx : dx);
            newDim.width = std::min(mMaxWinWidth,
                                    std::max(mMinWinWidth, newWidth));

            if (mMouseResize & LEFT)
            {
                newDim.x -= newDim.width - getWidth();
            }
        }

        // Keep guichan window inside screen (supports resizing any side)
        if (newDim.x < 0)
        {
            newDim.width += newDim.x;
            newDim.x = 0;
        }
        if (newDim.y < 0)
        {
            newDim.height += newDim.y;
            newDim.y = 0;
        }
        if (newDim.x + newDim.width > windowContainer->getWidth())
        {
            newDim.width = windowContainer->getWidth() - newDim.x;
        }
        if (newDim.y + newDim.height > windowContainer->getHeight())
        {
            newDim.height = windowContainer->getHeight() - newDim.y;
        }

        // Update mouse offset when dragging bottom or right border
        if (mMouseResize & BOTTOM)
        {
            mDragOffsetY += newDim.height - getHeight();
        }
        if (mMouseResize & RIGHT)
        {
            mDragOffsetX += newDim.width - getWidth();
        }

        // Set the new window and content dimensions
        setDimension(newDim);
        const gcn::Rectangle area = getChildrenArea();
        mChrome->setSize(area.width, area.height);
        updateContentSize();
    }
}

void Window::loadWindowState(std::string const &name)
{
    mConfigName = name;

    setPosition((int) config.getValue(name + "WinX", getX()),
                (int) config.getValue(name + "WinY", getY()));

    if (mResizable)
    {
        setSize((int) config.getValue(name + "WinWidth", getWidth()),
                (int) config.getValue(name + "WinHeight", getHeight()));

        const gcn::Rectangle area = getChildrenArea();
        mChrome->setSize(area.width, area.height);
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
    updateContentSize();
}
