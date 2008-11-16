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

#include <algorithm>
#include <cassert>
#include <climits>

#include <guichan/exception.hpp>
#include <guichan/widgets/icon.hpp>

#include <libxml/tree.h>

#include "window.h"

#include "gui.h"
#include "gccontainer.h"
#include "windowcontainer.h"

#include "widgets/layout.h"
#include "widgets/resizegrip.h"

#include "../configlistener.h"
#include "../configuration.h"
#include "../graphics.h"
#include "../log.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

#include "../utils/xml.h"

ConfigListener *Window::windowConfigListener = 0;
WindowContainer *Window::windowContainer = 0;
int Window::instances = 0;
int Window::mouseResize = 0;
Image *Window::closeImage = NULL;
bool mLoaded = false;
bool Window::mAlphaChanged = false;

class WindowConfigListener : public ConfigListener
{
    /*
    void optionChanged(const std::string &)
    {
        for_each(Window::border.grid, Window::border.grid + 9,
                std::bind2nd(std::mem_fun(&Image::setAlpha),
                    config.getValue("guialpha", 0.8)));
    }
    */

    void optionChanged(const std::string &)
    {
        Window::mAlphaChanged = true;
    }
};

Window::Window(const std::string& caption, bool modal, Window *parent):
    gcn::Window(caption),
    mGrip(0),
    mParent(parent),
    mLayout(NULL),
    mModal(modal),
    mCloseButton(false),
    mSticky(false),
    mMinWinWidth(100),
    mMinWinHeight(40),
    mMaxWinWidth(INT_MAX),
    mMaxWinHeight(INT_MAX)
{
    logger->log("Window::Window(\"%s\")", caption.c_str());

    if (!windowContainer)
    {
        throw GCN_EXCEPTION("Window::Window(): no windowContainer set");
    }

    loadSkin("graphics/gui/gui.xml");

    //if (instances == 0)
    //{
        //WindowConfigListener = new WindowConfigListener();
        // Send GUI alpha changed for initialization
        //windowConfigListener->optionChanged("guialpha");
        //config.addListener("guialpha", windowConfigListener);
    //}

    instances++;

    setFrameSize(0);
    setPadding(3);
    setTitleBarHeight(20);

    setGuiAlpha();

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
    logger->log("UNLOAD: Window::~Window(\"%s\")", getCaption().c_str());

    std::string const &name = mConfigName;
    if (!name.empty())
    {
        // Saving X, Y and Width and Height for resizables in the config
        config.setValue(name + "WinX", getX());
        config.setValue(name + "WinY", getY());

        if (mGrip)
        {
            config.setValue(name + "WinWidth", getWidth());
            config.setValue(name + "WinHeight", getHeight());
        }
    }

    delete mLayout;

    while (!mWidgets.empty())
    {
        gcn::Widget *w = mWidgets.front();
        remove(w);
        delete(w);
    }

    instances--;

    if (instances == 0)
    {
        config.removeListener("guialpha", windowConfigListener);
        delete windowConfigListener;
        windowConfigListener = NULL;

        // Clean up static resources
        closeImage->decRef();
    }

    // Clean up Border images.
    for( int i = 0; i < 9; i++ )
    {
        delete border[i];
        border[i] = NULL;
    }
}

void Window::setWindowContainer(WindowContainer *wc)
{
    windowContainer = wc;
}

void Window::draw(gcn::Graphics *graphics)
{
    if(mAlphaChanged)
        setGuiAlpha();


    Graphics *g = static_cast<Graphics*>(graphics);

    //g->drawImageRect(0, 0, getWidth(), getHeight(), border);

    g->drawImageRect(0, 0, getWidth(), getHeight(), border[0], border[2], border[6], border[8], border[1], border[5], border[7], border[3], border[4]);

    // Draw title
    if (getTitleBarHeight())
    {
        g->setColor(gcn::Color(0, 0, 0));
        g->setFont(getFont());
        g->drawText(getCaption(), 7, 5, gcn::Graphics::LEFT);
    }

    // Draw Close Button
    if (mCloseButton)
    {
        g->drawImage(closeImage,
            getWidth() - closeImage->getWidth() - getPadding(),
            getPadding()
        );
    }
    drawChildren(graphics);
}

void Window::setContentSize(int width, int height)
{
    setSize(width + 2 * getPadding(),
            height + getPadding() + getTitleBarHeight());
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
    if ((bool)mGrip == r) return;

    if (r)
    {
        mGrip = new ResizeGrip();
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
    if (mGrip)
    {
        gcn::Rectangle const &area = getChildrenArea();
        mGrip->setPosition(getWidth() - mGrip->getWidth() - area.x,
                           getHeight() - mGrip->getHeight() - area.y);
    }

    if (mLayout)
    {
        int w = getWidth() - 2 * getPadding();
        int h = getHeight() - getPadding() - getTitleBarHeight();
        mLayout->reflow(w, h);
    }
}

void Window::setCloseButton(bool flag)
{
    mCloseButton = flag;
}

bool Window::isResizable()
{
    return mGrip;
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
    gcn::Window::setVisible(isSticky() || visible);
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
                getWidth() - closeImage->getWidth() - getPadding(),
                getPadding(),
                closeImage->getWidth(),
                closeImage->getHeight());

            if (closeButtonRect.isPointInRect(x, y))
            {
                setVisible(false);
            }
        }

        // Handle window resizing
        mouseResize = getResizeHandles(event);
    }
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
    {
        gui->setCursorType(Gui::CURSOR_POINTER);
    }
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
        newX = std::min(windowContainer->getWidth() - getWidth(), newX);
        newY = std::min(windowContainer->getHeight() - getHeight(), newY);
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
            {
                newDim.y -= newDim.height - getHeight();
            }
        }

        if (mouseResize & (LEFT | RIGHT))
        {
            int newWidth = newDim.width + ((mouseResize & LEFT) ? -dx : dx);
            newDim.width = std::min(mMaxWinWidth,
                                    std::max(mMinWinWidth, newWidth));

            if (mouseResize & LEFT)
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

void Window::loadWindowState(std::string const &name)
{
    mConfigName = name;

    setPosition((int) config.getValue(name + "WinX", mDefaultX),
                (int) config.getValue(name + "WinY", mDefaultY));

    if (mGrip)
    {
        setSize((int) config.getValue(name + "WinWidth", mDefaultWidth),
                (int) config.getValue(name + "WinHeight", mDefaultHeight));
    }
    else
    {
        setSize(mDefaultWidth, mDefaultHeight);
    }
}

void Window::setDefaultSize(int defaultX, int defaultY,
                            int defaultWidth, int defaultHeight)
{
    mDefaultX = defaultX;
    mDefaultY = defaultY;
    mDefaultWidth = defaultWidth;
    mDefaultHeight = defaultHeight;
}

void Window::resetToDefaultSize()
{
    setPosition(mDefaultX, mDefaultY);
    setContentSize(mDefaultWidth, mDefaultHeight);
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

Layout &Window::getLayout()
{
    if (!mLayout) mLayout = new Layout;
    return *mLayout;
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

void Window::setGuiAlpha()
{
    //logger->log("Window::setGuiAlpha: Alpha Value %f", config.getValue("guialpha", 0.8));
    for(int i = 0; i < 9; i++)
    {
        //logger->log("Window::setGuiAlpha: Border Image (%i)", i);
        border[i]->setAlpha(config.getValue("guialpha", 0.8));
    }

    mAlphaChanged = false;
}

void Window::loadSkin(const std::string &fileName)
{
    ResourceManager *resman = ResourceManager::getInstance();

    logger->log("Loading Window Skin '%s'.", fileName.c_str());
    logger->log("Loading Window ID '%s'.", Window::getId().c_str());


    if (fileName == "")
        logger->error("Window::loadSkin(): Invalid File Name.");

    // TODO:
    // If there is an error loading the specified file, we should try to revert
    // to a 'default' skin file. Only if the 'default' skin file can't be loaded
    // should we have a terminating error.
    XML::Document doc(fileName);
    xmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlStrEqual(rootNode->name, BAD_CAST "skinset"))
    {
        logger->error("Widget Skinning error");
    }

    std::string skinSetImage;
    skinSetImage = XML::getProperty(rootNode, "image", "");
    Image *dBorders = NULL;
    if(skinSetImage != "")
    {
        logger->log("Window::loadSkin(): <skinset> defines '%s' as a skin image.", skinSetImage.c_str());
        dBorders = resman->getImage("graphics/gui/" + skinSetImage);//"graphics/gui/speech_bubble.png");
    }
    else
    {
        logger->error("Window::loadSkin(): Skinset does not define an image!");
    }

    //iterate <widget>'s
    for_each_xml_child_node(widgetNode, rootNode)
    {
        if (!xmlStrEqual(widgetNode->name, BAD_CAST "widget"))
            continue;

        std::string widgetType;
        widgetType = XML::getProperty(widgetNode, "type", "unknown");
        if (widgetType == "Window")
        {
            // Itarate through <part>'s
            // LEEOR / TODO:
            // We need to make provisions to load in a CloseButton image. For now it
            // can just be hard-coded.
            for_each_xml_child_node(partNode, widgetNode)
            {
                if (!xmlStrEqual(partNode->name, BAD_CAST "part"))
                {
                    continue;
                }

                std::string partType;
                partType = XML::getProperty(partNode, "type", "unknown");
                // TOP ROW
                if(partType == "top-left-corner")
                {
                    const int xPos = XML::getProperty(partNode, "xpos", 0);
                    const int yPos = XML::getProperty(partNode, "ypos", 0);
                    const int width = XML::getProperty(partNode, "width", 1);
                    const int height = XML::getProperty(partNode, "height", 1);

                    border[0] = dBorders->getSubImage(xPos, yPos, width, height);
                }
                else if(partType == "top-edge")
                {
                    const int xPos = XML::getProperty(partNode, "xpos", 0);
                    const int yPos = XML::getProperty(partNode, "ypos", 0);
                    const int width = XML::getProperty(partNode, "width", 1);
                    const int height = XML::getProperty(partNode, "height", 1);

                    border[1] = dBorders->getSubImage(xPos, yPos, width, height);
                }
                else if(partType == "top-right-corner")
                {
                    const int xPos = XML::getProperty(partNode, "xpos", 0);
                    const int yPos = XML::getProperty(partNode, "ypos", 0);
                    const int width = XML::getProperty(partNode, "width", 1);
                    const int height = XML::getProperty(partNode, "height", 1);

                    border[2] = dBorders->getSubImage(xPos, yPos, width, height);
                }

                // MIDDLE ROW
                else if(partType == "left-edge")
                {
                    const int xPos = XML::getProperty(partNode, "xpos", 0);
                    const int yPos = XML::getProperty(partNode, "ypos", 0);
                    const int width = XML::getProperty(partNode, "width", 1);
                    const int height = XML::getProperty(partNode, "height", 1);

                    border[3] = dBorders->getSubImage(xPos, yPos, width, height);
                }
                else if(partType == "bg-quad")
                {
                    const int xPos = XML::getProperty(partNode, "xpos", 0);
                    const int yPos = XML::getProperty(partNode, "ypos", 0);
                    const int width = XML::getProperty(partNode, "width", 1);
                    const int height = XML::getProperty(partNode, "height", 1);

                    border[4] = dBorders->getSubImage(xPos, yPos, width, height);
                }
                else if(partType == "right-edge")
                {
                    const int xPos = XML::getProperty(partNode, "xpos", 0);
                    const int yPos = XML::getProperty(partNode, "ypos", 0);
                    const int width = XML::getProperty(partNode, "width", 1);
                    const int height = XML::getProperty(partNode, "height", 1);

                    border[5] = dBorders->getSubImage(xPos, yPos, width, height);
                }

                // BOTTOM ROW
                else if(partType == "bottom-left-corner")
                {
                    const int xPos = XML::getProperty(partNode, "xpos", 0);
                    const int yPos = XML::getProperty(partNode, "ypos", 0);
                    const int width = XML::getProperty(partNode, "width", 1);
                    const int height = XML::getProperty(partNode, "height", 1);

                    border[6] = dBorders->getSubImage(xPos, yPos, width, height);
                }
                else if(partType == "bottom-edge")
                {
                    const int xPos = XML::getProperty(partNode, "xpos", 0);
                    const int yPos = XML::getProperty(partNode, "ypos", 0);
                    const int width = XML::getProperty(partNode, "width", 1);
                    const int height = XML::getProperty(partNode, "height", 1);

                    border[7] = dBorders->getSubImage(xPos, yPos, width, height);
                }
                else if(partType == "bottom-right-corner")
                {
                    const int xPos = XML::getProperty(partNode, "xpos", 0);
                    const int yPos = XML::getProperty(partNode, "ypos", 0);
                    const int width = XML::getProperty(partNode, "width", 1);
                    const int height = XML::getProperty(partNode, "height", 1);

                    border[8] = dBorders->getSubImage(xPos, yPos, width, height);
                }

                // Part is of an uknown type.
                else
                {
                    logger->log("Window::loadSkin(): Unknown Part Type '%s'", partType.c_str());
                }
            }
        }
        // Widget is of an uknown type.
        else
        {
            logger->log("Window::loadSkin(): Unknown Widget Type '%s'", widgetType.c_str());
        }
    }
    dBorders->decRef();

    logger->log("Finished loading Window Skin.");

    // Hard-coded for now until we update the above code to look for window buttons.
    closeImage = resman->getImage("graphics/gui/close_button.png");
}
