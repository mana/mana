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
#include "textrenderer.h"

#include "gui/gui.h"
#include "gui/viewport.h"

#include "gui/widgets/layout.h"
#include "gui/widgets/resizegrip.h"
#include "gui/widgets/windowcontainer.h"

#include <guichan/exception.hpp>
#include <guichan/focushandler.hpp>

#include <algorithm>
#include <cassert>

int Window::instances = 0;
int Window::mouseResize = 0;

Window::Window(const std::string &caption, bool modal, Window *parent)
    : Window(SkinType::Window, caption, modal, parent)
{}

Window::Window(SkinType skinType, const std::string &caption, bool modal, Window *parent)
    : gcn::Window(caption)
    , mParent(parent)
    , mModal(modal)
    , mSkinType(skinType)
    , mMaxWinWidth(graphics->getWidth())
    , mMaxWinHeight(graphics->getHeight())
{
    Log::debug("Window::Window(\"%s\")", caption.c_str());

    if (!windowContainer)
        throw GCN_EXCEPTION("Window::Window(): no windowContainer set");

    instances++;

    auto &skin = getSkin();
    setFrameSize(skin.frameSize);
    setPadding(skin.padding);
    setTitleBarHeight(skin.titleBarHeight);

    // Add this window to the window container
    windowContainer->add(this);

    if (mModal)
    {
        gui->setCursorType(Cursor::Pointer);
        requestModalFocus();
    }

    // Windows are invisible by default
    setVisible(false);

    addWidgetListener(this);
}

Window::~Window()
{
    Log::debug("Window::~Window(\"%s\")", getCaption().c_str());

    saveWindowState();

    delete mLayout;

    while (!mWidgets.empty())
        delete mWidgets.front();

    removeWidgetListener(this);

    instances--;
}

void Window::setWindowContainer(WindowContainer *wc)
{
    windowContainer = wc;
}

void Window::draw(gcn::Graphics *graphics)
{
    if (getFrameSize() == 0)
        drawFrame(graphics);

    auto g = static_cast<Graphics*>(graphics);

    if (mCloseButton)
    {
        WidgetState state(getCloseButtonRect(), mCloseButtonHovered ? STATE_HOVERED : 0);
        gui->getTheme()->drawSkin(g, SkinType::ButtonClose, state);
    }

    if (mStickyButton)
    {
        WidgetState state(getStickyButtonRect(), mSticky ? STATE_SELECTED : 0);
        gui->getTheme()->drawSkin(g, SkinType::ButtonSticky, state);
    }

    drawChildren(graphics);
}

void Window::drawFrame(gcn::Graphics *graphics)
{
    auto g = static_cast<Graphics*>(graphics);

    WidgetState widgetState(this);
    widgetState.width += getFrameSize() * 2;
    widgetState.height += getFrameSize() * 2;

    auto &skin = getSkin();
    skin.draw(g, widgetState);

    if (mShowTitle)
    {
        if (auto skinState = skin.getState(widgetState.flags))
        {
            auto &textFormat = skinState->textFormat;
            TextRenderer::renderText(g,
                                     getCaption(),
                                     getFrameSize() + skin.titleOffsetX,
                                     getFrameSize() + skin.titleOffsetY,
                                     gcn::Graphics::LEFT,
                                     textFormat.bold ? boldFont : getFont(),
                                     textFormat);
        }
    }
}

void Window::setContentSize(int width, int height)
{
    width += 2 * getPadding();
    height += getPadding() + getTitleBarHeight();

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

void Window::setMinimumContentSize(int width, int height)
{
    const int padding = getPadding();
    const int titleBarHeight = getTitleBarHeight();
    auto &skin = getSkin();

    setMinWidth(std::max(skin.getMinWidth(), width + 2 * padding));
    setMinHeight(std::max(skin.getMinHeight(), height + padding + titleBarHeight));
}

void Window::setLocationRelativeTo(gcn::Widget *widget)
{
    int wx;
    int wy;
    widget->getAbsolutePosition(wx, wy);

    int x;
    int y;
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
    mMinWinWidth = std::max(getSkin().getMinWidth(), width);
}

void Window::setMinHeight(int height)
{
    mMinWinHeight = std::max(getSkin().getMinHeight(), height);
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
        mGrip = nullptr;
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
        gui->setCursorType(Cursor::Pointer);

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

        if (mCloseButton && getCloseButtonRect().isPointInRect(x, y))
            close();

        if (mStickyButton && getStickyButtonRect().isPointInRect(x, y))
            setSticky(!isSticky());

        // Update resizing state and disable moving if we're resizing the window
        mouseResize = getResizeHandles(event);
        if (mouseResize)
            mMoved = false;
    }
}

void Window::close()
{
    setVisible(false);
}

const Skin &Window::getSkin() const
{
    return gui->getTheme()->getSkin(mSkinType);
}

void Window::mouseReleased(gcn::MouseEvent &event)
{
    mouseResize = 0;

    gcn::Window::mouseReleased(event);
}

void Window::mouseExited(gcn::MouseEvent &event)
{
    if (mGrip && !mouseResize)
        gui->setCursorType(Cursor::Pointer);

    mCloseButtonHovered = false;
}

void Window::mouseMoved(gcn::MouseEvent &event)
{
    mCloseButtonHovered = false;

    // Make sure BeingPopup is hidden (Viewport does not receive mouseExited)
    if (viewport)
        viewport->hideBeingPopup();

    // Don't change mouse cursor when event was consumed by child widget
    // (in this case child widget is responsible for mouse cursor)
    if (event.isConsumed())
        return;

    mCloseButtonHovered = getCloseButtonRect().isPointInRect(event.getX(), event.getY());
    Cursor cursor = Cursor::Pointer;

    // Changes the custom mouse cursor based on its current position.
    if (!mCloseButtonHovered)
    {
        switch (getResizeHandles(event))
        {
        case BOTTOM | RIGHT:
        case TOP | LEFT:
            cursor = Cursor::ResizeDownRight;
            break;
        case BOTTOM | LEFT:
        case TOP | RIGHT:
            cursor = Cursor::ResizeDownLeft;
            break;
        case BOTTOM:
        case TOP:
            cursor = Cursor::ResizeDown;
            break;
        case RIGHT:
        case LEFT:
            cursor = Cursor::ResizeAcross;
            break;
        default:
            break;
        }
    }

    gui->setCursorType(cursor);
}

void Window::mouseDragged(gcn::MouseEvent &event)
{
    // Let Guichan handle title bar drag
    gcn::Window::mouseDragged(event);

    // Keep guichan window inside screen when it may be moved
    if (isMovable() && mMoved)
        ensureOnScreen();

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
    assert(!mWindowName.empty());

    constexpr WindowState defaultState;
    auto s = config.windows.find(mWindowName);
    const WindowState state = s == config.windows.end() ? defaultState
                                                        : s->second;

    setPosition(state.x.value_or(mDefaultX),
                state.y.value_or(mDefaultY));

    if (mSaveVisible)
        setVisible(state.visible.value_or(mDefaultVisible));

    if (mStickyButton)
        setSticky(state.sticky.value_or(isSticky()));

    if (mGrip)
    {
        const int width = state.width.value_or(mDefaultWidth);
        const int height = state.height.value_or(mDefaultHeight);

        setSize(std::clamp(width, getMinWidth(), getMaxWidth()),
                std::clamp(height, getMinHeight(), getMaxHeight()));
    }
    else
    {
        setSize(mDefaultWidth, mDefaultHeight);
    }

    // Check if the window is off screen...
    ensureOnScreen();
}

void Window::saveWindowState() const
{
    // Saving X, Y and Width and Height for resizables in the config
    if (mWindowName.empty())
        return;

    WindowState state;
    state.x = getX();
    state.y = getY();

    if (mSaveVisible)
        state.visible = isVisible();

    if (mStickyButton)
        state.sticky = isSticky();

    if (mGrip)
    {
        state.width = getWidth();
        state.height = getHeight();
    }

    config.windows[mWindowName] = state;
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
    int x = 0;
    int y = 0;

    switch (position)
    {
    case ImageRect::UPPER_LEFT:
        break;
    case ImageRect::UPPER_CENTER:
        x = (graphics->getWidth() - defaultWidth) / 2;
        break;
    case ImageRect::UPPER_RIGHT:
        x = graphics->getWidth() - defaultWidth;
        break;
    case ImageRect::LEFT:
        y = (graphics->getHeight() - defaultHeight) / 2;
        break;
    case ImageRect::CENTER:
        x = (graphics->getWidth() - defaultWidth) / 2;
        y = (graphics->getHeight() - defaultHeight) / 2;
        break;
    case ImageRect::RIGHT:
        x = graphics->getWidth() - defaultWidth;
        y = (graphics->getHeight() - defaultHeight) / 2;
        break;
    case ImageRect::LOWER_LEFT:
        y = graphics->getHeight() - defaultHeight;
        break;
    case ImageRect::LOWER_CENTER:
        x = (graphics->getWidth() - defaultWidth) / 2;
        y = graphics->getHeight() - defaultHeight;
        break;
    case ImageRect::LOWER_RIGHT:
        x = graphics->getWidth() - defaultWidth;
        y = graphics->getHeight() - defaultHeight;
        break;
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

    if (mGrip)
    {
        const int x = event.getX();
        const int y = event.getY();
        const int p = getPadding();

        const bool inPadding = (x < p || x >= getWidth() - p) ||
                               (y < p || y >= getHeight() - p);

        if (inPadding && event.getSource() == this)
        {
            /**
             * The width of the resize border. Is independent of the actual window
             * border width, and determines mostly the size of the corner area
             * where two borders are moved at the same time.
             */
            const int resizeBorderWidth = std::max(mGrip->getWidth(), 10);

            resizeHandles |= (x >= getWidth() - resizeBorderWidth) ? RIGHT :
                              (x < resizeBorderWidth) ? LEFT : 0;
            resizeHandles |= (y >= getHeight() - resizeBorderWidth) ? BOTTOM :
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

gcn::Rectangle Window::getCloseButtonRect() const
{
    if (!mCloseButton)
        return {};

    auto theme = gui->getTheme();

    auto &closeSkin = theme->getSkin(SkinType::ButtonClose);
    const int closeWidth = closeSkin.getMinWidth();
    const int closeHeight = closeSkin.getMinHeight();

    return {
        getWidth() - closeWidth - closeSkin.padding,
        closeSkin.padding,
        closeWidth,
        closeHeight
    };
}

gcn::Rectangle Window::getStickyButtonRect() const
{
    if (!mStickyButton)
        return {};

    auto theme = gui->getTheme();

    auto &closeSkin = theme->getSkin(SkinType::ButtonClose);
    const int closeWidth = closeSkin.getMinWidth();

    auto &stickySkin = theme->getSkin(SkinType::ButtonSticky);
    const int stickyWidth = stickySkin.getMinWidth();
    const int stickyHeight = stickySkin.getMinHeight();

    int stickyX = getWidth() - stickyWidth - stickySkin.padding - stickySkin.spacing;
    if (mCloseButton)
        stickyX -= closeWidth + closeSkin.padding;

    return {
        stickyX,
        stickySkin.padding,
        stickyWidth,
        stickyHeight
    };
}

int Window::getGuiAlpha()
{
    float alpha = std::max(config.guiAlpha,
                           gui->getTheme()->getMinimumOpacity());
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
    mLayout = nullptr;
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
    if (auto window = getParentWindow())
        setLocationRelativeTo(window);
    else
        setLocationRelativeTo(getParent());
}

void Window::ensureOnScreen()
{
    // Skip when a window hasn't got any size initialized yet
    if (getWidth() == 0 && getHeight() == 0)
        return;

    gcn::Rectangle dim = getDimension();

    // Check the left and bottom screen boundaries
    dim.x = std::min(graphics->getWidth() - dim.width, dim.x);
    dim.y = std::min(graphics->getHeight() - dim.height, dim.y);

    // But never allow the windows to disappear in to the right and top
    dim.x = std::max(0, dim.x);
    dim.y = std::max(0, dim.y);

    setPosition(dim.x, dim.y);
}
