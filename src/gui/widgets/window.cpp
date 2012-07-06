/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/widgets/window.h"

#include "configuration.h"
#include "log.h"

#include "gui/gui.h"
#include "gui/palette.h"
#include "gui/viewport.h"

#include "gui/widgets/layout.h"
#include "gui/widgets/resizegrip.h"
#include "gui/widgets/windowcontainer.h"

#include "resources/image.h"
#include "resources/theme.h"

#include <guichan/exception.hpp>
#include <guichan/focushandler.hpp>

int Window::instances = 0;
int Window::mouseResize = 0;

Window::Window(const std::string &caption, bool modal, Window *parent,
               const std::string &skin):
    gcn::Window(caption),
    mGrip(0),
    mParent(parent),
    mLayout(NULL),
    mWindowName("window"),
    mShowTitle(true),
    mModal(modal),
    mCloseButton(false),
    mDefaultVisible(false),
    mSaveVisible(false),
    mStickyButton(false),
    mSticky(false),
    mMinWinWidth(100),
    mMinWinHeight(40),
    mMaxWinWidth(graphics->getWidth()),
    mMaxWinHeight(graphics->getHeight())
{
    logger->log("Window::Window(\"%s\")", caption.c_str());

    if (!windowContainer)
        throw GCN_EXCEPTION("Window::Window(): no windowContainer set");

    instances++;

    setFrameSize(0);
    setPadding(5);
    setTitleBarHeight(25);

    // Loads the skin
    mSkin = Theme::instance()->load(skin);

    // Add this window to the window container
    windowContainer->add(this);

    if (mModal)
    {
        gui->setCursorType(Gui::CURSOR_POINTER);
        requestModalFocus();
    }

    // Windows are invisible by default
    setVisible(false);

    addWidgetListener(this);
}

Window::~Window()
{
    logger->log("Window::~Window(\"%s\")", getCaption().c_str());

    saveWindowState();

    delete mLayout;

    while (!mWidgets.empty())
        delete mWidgets.front();

    removeWidgetListener(this);

    instances--;

    mSkin->instances--;
}

void Window::setWindowContainer(WindowContainer *wc)
{
    windowContainer = wc;
}

void Window::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics*>(graphics);

    g->drawImageRect(0, 0, getWidth(), getHeight(), mSkin->getBorder());

    // Draw title
    if (mShowTitle)
    {
        g->setColor(Theme::getThemeColor(Theme::TEXT));
        g->setFont(getFont());
        g->drawText(getCaption(), 7, 10, gcn::Graphics::LEFT);
    }

    // Draw Close Button
    if (mCloseButton)
    {
        g->drawImage(mSkin->getCloseImage(),
            getWidth() - mSkin->getCloseImage()->getWidth() - getPadding(),
            getPadding());
    }

    // Draw Sticky Button
    if (mStickyButton)
    {
        Image *button = mSkin->getStickyImage(mSticky);
        int x = getWidth() - button->getWidth() - getPadding();
        if (mCloseButton)
            x -= mSkin->getCloseImage()->getWidth();

        g->drawImage(button, x, getPadding());
    }

    drawChildren(graphics);
}

void Window::setContentSize(int width, int height)
{
    width = width + 2 * getPadding();
    height = height + getPadding() + getTitleBarHeight();

    if (getMinWidth() > width)
        width = getMinWidth();
    else if (getMaxWidth() < width)
        width = getMaxWidth();
    if (getMinHeight() > height)
        height = getMinHeight();
    else if (getMaxHeight() < height)
        height = getMaxHeight();

    setSize(width, height);
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

void Window::setLocationRelativeTo(ImageRect::ImagePosition position,
                                   int offsetX, int offsetY)
{
    if (position == ImageRect::UPPER_LEFT)
    {
    }
    else if (position == ImageRect::UPPER_CENTER)
    {
        offsetX += (graphics->getWidth() - getWidth()) / 2;
    }
    else if (position == ImageRect::UPPER_RIGHT)
    {
        offsetX += graphics->getWidth() - getWidth();
    }
    else if (position == ImageRect::LEFT)
    {
        offsetY += (graphics->getHeight() - getHeight()) / 2;
    }
    else if (position == ImageRect::CENTER)
    {
        offsetX += (graphics->getWidth() - getWidth()) / 2;
        offsetY += (graphics->getHeight() - getHeight()) / 2;
    }
    else if (position == ImageRect::RIGHT)
    {
        offsetX += graphics->getWidth() - getWidth();
        offsetY += (graphics->getHeight() - getHeight()) / 2;
    }
    else if (position == ImageRect::LOWER_LEFT)
    {
        offsetY += graphics->getHeight() - getHeight();
    }
    else if (position == ImageRect::LOWER_CENTER)
    {
        offsetX += (graphics->getWidth() - getWidth()) / 2;
        offsetY += graphics->getHeight() - getHeight();
    }
    else if (position == ImageRect::LOWER_RIGHT)
    {
        offsetX += graphics->getWidth() - getWidth();
        offsetY += graphics->getHeight() - getHeight();
    }

    setPosition(offsetX, offsetY);
}

void Window::setMinWidth(int width)
{
    mMinWinWidth = width > mSkin->getMinWidth() ? width : mSkin->getMinWidth();
}

void Window::setMinHeight(int height)
{
    mMinWinHeight = height > mSkin->getMinHeight() ?
                        height : mSkin->getMinHeight();
}

void Window::setMaxWidth(int width)
{
    mMaxWinWidth = width;
}

void Window::setMaxHeight(int height)
{
    mMaxWinHeight = height;
}

void Window::setResizable(bool r)
{
    if ((bool) mGrip == r)
        return;

    if (r)
    {
        mGrip = new ResizeGrip;
        mGrip->setX(getWidth() - mGrip->getWidth() - getChildrenArea().x);
        mGrip->setY(getHeight() - mGrip->getHeight() - getChildrenArea().y);
        add(mGrip);
    }
    else
    {
        remove(mGrip);
        delete mGrip;
        mGrip = 0;
    }
}

void Window::widgetResized(const gcn::Event &event)
{
    const gcn::Rectangle area = getChildrenArea();

    if (mGrip)
        mGrip->setPosition(getWidth() - mGrip->getWidth() - area.x,
                           getHeight() - mGrip->getHeight() - area.y);

    if (mLayout)
    {
        int w = area.width;
        int h = area.height;
        mLayout->reflow(w, h);
    }
}

void Window::widgetHidden(const gcn::Event &event)
{
    if (gui)
    {
        gui->setCursorType(Gui::CURSOR_POINTER);
    }

    WidgetListIterator it;

    for (it = mWidgets.begin(); it != mWidgets.end(); it++)
    {
        if (mFocusHandler->isFocused(*it))
            mFocusHandler->focusNone();
    }
}

void Window::setCloseButton(bool flag)
{
    mCloseButton = flag;
}

bool Window::isResizable() const
{
    return mGrip;
}

void Window::setStickyButton(bool flag)
{
    mStickyButton = flag;
}

void Window::setSticky(bool sticky)
{
    mSticky = sticky;
}

void Window::setVisible(bool visible)
{
    setVisible(visible, false);
}

void Window::setVisible(bool visible, bool forceSticky)
{
    if (visible == isVisible())
        return; // Nothing to do

    // Check if the window is off screen...
    if (visible)
        ensureOnScreen();

    gcn::Window::setVisible((!forceSticky && isSticky()) || visible);
}

void Window::scheduleDelete()
{
    windowContainer->scheduleDelete(this);
}

void Window::mousePressed(gcn::MouseEvent &event)
{
    // Let Guichan move window to top and figure out title bar drag
    gcn::Window::mousePressed(event);

    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        const int x = event.getX();
        const int y = event.getY();

        // Handle close button
        if (mCloseButton)
        {
            gcn::Rectangle closeButtonRect(
                getWidth() - mSkin->getCloseImage()->getWidth() - getPadding(),
                getPadding(),
                mSkin->getCloseImage()->getWidth(),
                mSkin->getCloseImage()->getHeight());

            if (closeButtonRect.isPointInRect(x, y))
            {
                close();
            }
        }

        // Handle sticky button
        if (mStickyButton)
        {
            Image *button = mSkin->getStickyImage(mSticky);
            int rx = getWidth() - button->getWidth() - getPadding();
            if (mCloseButton)
                rx -= mSkin->getCloseImage()->getWidth();
            gcn::Rectangle stickyButtonRect(rx, getPadding(),
                                    button->getWidth(), button->getHeight());

            if (stickyButtonRect.isPointInRect(x, y))
            {
                setSticky(!isSticky());
            }
        }

        // Handle window resizing
        mouseResize = getResizeHandles(event);
    }
}

void Window::close()
{
    setVisible(false);
}

void Window::mouseReleased(gcn::MouseEvent &event)
{
    if (mGrip && mouseResize)
    {
        mouseResize = 0;
        gui->setCursorType(Gui::CURSOR_POINTER);
    }

    // This should be the responsibility of Guichan (and is from 0.8.0 on)
    mMoved = false;
}

void Window::mouseExited(gcn::MouseEvent &event)
{
    if (mGrip && !mouseResize)
        gui->setCursorType(Gui::CURSOR_POINTER);
}

void Window::mouseMoved(gcn::MouseEvent &event)
{
    int resizeHandles = getResizeHandles(event);

    // Changes the custom mouse cursor based on it's current position.
    switch (resizeHandles)
    {
        case BOTTOM | RIGHT:
            gui->setCursorType(Gui::CURSOR_RESIZE_DOWN_RIGHT);
            break;
        case BOTTOM | LEFT:
            gui->setCursorType(Gui::CURSOR_RESIZE_DOWN_LEFT);
            break;
        case BOTTOM:
            gui->setCursorType(Gui::CURSOR_RESIZE_DOWN);
            break;
        case RIGHT:
        case LEFT:
            gui->setCursorType(Gui::CURSOR_RESIZE_ACROSS);
            break;
        default:
            gui->setCursorType(Gui::CURSOR_POINTER);
    }

    if (viewport)
        viewport->hideBeingPopup();
}

void Window::mouseDragged(gcn::MouseEvent &event)
{
    // Let Guichan handle title bar drag
    gcn::Window::mouseDragged(event);

    // Keep guichan window inside screen when it may be moved
    if (isMovable() && mMoved)
    {
        int newX = std::max(0, getX());
        int newY = std::max(0, getY());
        newX = std::min(graphics->getWidth() - getWidth(), newX);
        newY = std::min(graphics->getHeight() - getHeight(), newY);
        setPosition(newX, newY);
    }

    if (mouseResize && !mMoved)
    {
        const int dx = event.getX() - mDragOffsetX;
        const int dy = event.getY() - mDragOffsetY;
        gcn::Rectangle newDim = getDimension();

        if (mouseResize & (TOP | BOTTOM))
        {
            int newHeight = newDim.height + ((mouseResize & TOP) ? -dy : dy);
            newDim.height = std::min(mMaxWinHeight,
                                     std::max(mMinWinHeight, newHeight));

            if (mouseResize & TOP)
                newDim.y -= newDim.height - getHeight();
        }

        if (mouseResize & (LEFT | RIGHT))
        {
            int newWidth = newDim.width + ((mouseResize & LEFT) ? -dx : dx);
            newDim.width = std::min(mMaxWinWidth,
                                    std::max(mMinWinWidth, newWidth));

            if (mouseResize & LEFT)
                newDim.x -= newDim.width - getWidth();
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
        if (newDim.x + newDim.width > graphics->getWidth())
        {
            newDim.width = graphics->getWidth() - newDim.x;
        }
        if (newDim.y + newDim.height > graphics->getHeight())
        {
            newDim.height = graphics->getHeight() - newDim.y;
        }

        // Update mouse offset when dragging bottom or right border
        if (mouseResize & BOTTOM)
        {
            mDragOffsetY += newDim.height - getHeight();
        }
        if (mouseResize & RIGHT)
        {
            mDragOffsetX += newDim.width - getWidth();
        }

        // Set the new window and content dimensions
        setDimension(newDim);
    }
}

void Window::loadWindowState()
{
    const std::string &name = mWindowName;
    assert(!name.empty());

    setPosition((int) config.getValue(name + "WinX", mDefaultX),
                (int) config.getValue(name + "WinY", mDefaultY));

    if (mSaveVisible)
        setVisible((bool) config.getValue(name + "Visible", mDefaultVisible));

    if (mStickyButton)
        setSticky((bool) config.getValue(name + "Sticky", isSticky()));

    if (mGrip)
    {
        int width = (int) config.getValue(name + "WinWidth", mDefaultWidth);
        int height = (int) config.getValue(name + "WinHeight", mDefaultHeight);

        if (getMinWidth() > width)
            width = getMinWidth();
        else if (getMaxWidth() < width)
            width = getMaxWidth();
        if (getMinHeight() > height)
            height = getMinHeight();
        else if (getMaxHeight() < height)
            height = getMaxHeight();

        setSize(width, height);
    }
    else
    {
        setSize(mDefaultWidth, mDefaultHeight);
    }

    // Check if the window is off screen...
    ensureOnScreen();
}

void Window::saveWindowState()
{
    // Saving X, Y and Width and Height for resizables in the config
    if (!mWindowName.empty() && mWindowName != "window")
    {
        config.setValue(mWindowName + "WinX", getX());
        config.setValue(mWindowName + "WinY", getY());

        if (mSaveVisible)
            config.setValue(mWindowName + "Visible", isVisible());

        if (mStickyButton)
            config.setValue(mWindowName + "Sticky", isSticky());

        if (mGrip)
        {
            if (getMinWidth() > getWidth())
                setWidth(getMinWidth());
            else if (getMaxWidth() < getWidth())
                setWidth(getMaxWidth());
            if (getMinHeight() > getHeight())
                setHeight(getMinHeight());
            else if (getMaxHeight() < getHeight())
                setHeight(getMaxHeight());

            config.setValue(mWindowName + "WinWidth", getWidth());
            config.setValue(mWindowName + "WinHeight", getHeight());
        }
    }
}

void Window::setDefaultSize(int defaultX, int defaultY,
                            int defaultWidth, int defaultHeight)
{
    if (getMinWidth() > defaultWidth)
        defaultWidth = getMinWidth();
    else if (getMaxWidth() < defaultWidth)
        defaultWidth = getMaxWidth();
    if (getMinHeight() > defaultHeight)
        defaultHeight = getMinHeight();
    else if (getMaxHeight() < defaultHeight)
        defaultHeight = getMaxHeight();

    mDefaultX = defaultX;
    mDefaultY = defaultY;
    mDefaultWidth = defaultWidth;
    mDefaultHeight = defaultHeight;
}

void Window::setDefaultSize()
{
    mDefaultX = getX();
    mDefaultY = getY();
    mDefaultWidth = getWidth();
    mDefaultHeight = getHeight();
}

void Window::setDefaultSize(int defaultWidth, int defaultHeight,
                            ImageRect::ImagePosition position,
                            int offsetX, int offsetY)
{
    int x = 0, y = 0;

    if (position == ImageRect::UPPER_LEFT)
    {
    }
    else if (position == ImageRect::UPPER_CENTER)
    {
        x = (graphics->getWidth() - defaultWidth) / 2;
    }
    else if (position == ImageRect::UPPER_RIGHT)
    {
        x = graphics->getWidth() - defaultWidth;
    }
    else if (position == ImageRect::LEFT)
    {
        y = (graphics->getHeight() - defaultHeight) / 2;
    }
    else if (position == ImageRect::CENTER)
    {
        x = (graphics->getWidth() - defaultWidth) / 2;
        y = (graphics->getHeight() - defaultHeight) / 2;
    }
    else if (position == ImageRect::RIGHT)
    {
        x = graphics->getWidth() - defaultWidth;
        y = (graphics->getHeight() - defaultHeight) / 2;
    }
    else if (position == ImageRect::LOWER_LEFT)
    {
        y = graphics->getHeight() - defaultHeight;
    }
    else if (position == ImageRect::LOWER_CENTER)
    {
        x = (graphics->getWidth() - defaultWidth) / 2;
        y = graphics->getHeight() - defaultHeight;
    }
    else if (position == ImageRect::LOWER_RIGHT)
    {
        x = graphics->getWidth() - defaultWidth;
        y = graphics->getHeight() - defaultHeight;
    }

    mDefaultX = x - offsetX;
    mDefaultY = y - offsetY;
    mDefaultWidth = defaultWidth;
    mDefaultHeight = defaultHeight;
}

void Window::resetToDefaultSize()
{
    setPosition(mDefaultX, mDefaultY);
    setSize(mDefaultWidth, mDefaultHeight);
    saveWindowState();
}

void Window::adjustPositionAfterResize(int oldScreenWidth, int oldScreenHeight)
{
    gcn::Rectangle dimension = getDimension();

    // If window was aligned to the right or bottom, keep it there
    const int rightMargin = oldScreenWidth - (getX() + getWidth());
    const int bottomMargin = oldScreenHeight - (getY() + getHeight());
    if (getX() > 0 && getX() > rightMargin)
        dimension.x = graphics->getWidth() - rightMargin - getWidth();
    if (getY() > 0 && getY() > bottomMargin)
        dimension.y = graphics->getHeight() - bottomMargin - getHeight();

    setDimension(dimension);
    ensureOnScreen();
}

int Window::getResizeHandles(gcn::MouseEvent &event)
{
    int resizeHandles = 0;
    const int y = event.getY();

    if (mGrip && y > (int) mTitleBarHeight)
    {
        const int x = event.getX();

        if (!getChildrenArea().isPointInRect(x, y) &&
                event.getSource() == this)
        {
            resizeHandles |= (x > getWidth() - resizeBorderWidth) ? RIGHT :
                              (x < resizeBorderWidth) ? LEFT : 0;
            resizeHandles |= (y > getHeight() - resizeBorderWidth) ? BOTTOM :
                              (y < resizeBorderWidth) ? TOP : 0;
        }

        if (event.getSource() == mGrip)
        {
            mDragOffsetX = x;
            mDragOffsetY = y;
            resizeHandles |= BOTTOM | RIGHT;
        }
    }

    return resizeHandles;
}

int Window::getGuiAlpha()
{
    float alpha = std::max(config.getFloatValue("guialpha"),
                           Theme::instance()->getMinimumOpacity());
    return (int) (alpha * 255.0f);
}

Layout &Window::getLayout()
{
    if (!mLayout)
        mLayout = new Layout;
    return *mLayout;
}

void Window::clearLayout()
{
    clear();

    // Restore the resize grip
    if (mGrip)
        add(mGrip);

    // Recreate layout instance when one is present
    if (mLayout)
    {
        delete mLayout;
        mLayout = new Layout;
    }
}

LayoutCell &Window::place(int x, int y, gcn::Widget *wg, int w, int h)
{
    add(wg);
    return getLayout().place(wg, x, y, w, h);
}

ContainerPlacer Window::getPlacer(int x, int y)
{
    return ContainerPlacer(this, &getLayout().at(x, y));
}

void Window::reflowLayout(int w, int h)
{
    assert(mLayout);
    mLayout->reflow(w, h);
    delete mLayout;
    mLayout = NULL;
    setContentSize(w, h);
}

void Window::redraw()
{
    if (mLayout)
    {
        const gcn::Rectangle area = getChildrenArea();
        int w = area.width;
        int h = area.height;
        mLayout->reflow(w, h);
    }
}

void Window::center()
{
    setLocationRelativeTo(getParent());
}

void Window::ensureOnScreen()
{
    // Skip when a window hasn't got any size initialized yet
    if (getWidth() == 0 && getHeight() == 0)
        return;

    gcn::Rectangle dimension = getDimension();

    // Check the left and bottom screen boundaries
    if (dimension.x + dimension.width > graphics->getWidth())
        dimension.x = graphics->getWidth() - dimension.width;
    if (dimension.y + dimension.height > graphics->getHeight())
        dimension.y = graphics->getHeight() - dimension.height;

    // But never allow the windows to disappear in to the right and top
    if (dimension.x < 0)
        dimension.x = 0;
    if (dimension.y < 0)
        dimension.y = 0;

    setDimension(dimension);
}
