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
#include "../graphic/graphic.h"
#include "../resources/resourcemanager.h"

WindowContainer *Window::windowContainer = NULL;

Window::Window(const std::string& text, bool modal, Window *parent):
    parent(parent),
    caption(text),
    mousePX(0),
    mousePY(0),
    snapSize(8),
    mouseDown(false),
    modal(modal),
    titlebarHeight(20)
{
    log("Window::Window(\"%s\")", caption.c_str());

    titlebarColor.r = 203;
    titlebarColor.g = 203;
    titlebarColor.b = 203;

    setBaseColor(gcn::Color(255, 255, 255));

    // Load dialog title bar image
    ResourceManager *resman = ResourceManager::getInstance();
    dBorders = resman->getImage("core/graphics/gui/vscroll_grey.png");
    dBackground = resman->getImage("core/graphics/gui/bg_quad_dis.png");
    dWindowSurface = 0;
    
    dWindowSurface = SDL_AllocSurface(SDL_SWSURFACE, 20, 20, (screen->format->BytesPerPixel*8), 0, 0, 0, 0);
    Uint32 boxColor = SDL_MapRGB(screen->format, 255, 0, 255);
    SDL_Rect sourceRect;
    sourceRect.x = sourceRect.y = 0;
    sourceRect.w = 20;
    sourceRect.h = 20;
    if ( dWindowSurface )
    { 
        SDL_FillRect(dWindowSurface, &sourceRect, boxColor);
        //SDL_SetAlpha(chatBoxBackground, SDL_SRCALPHA, 120);
    }

    // Register mouse listener
    addMouseListener(this);

    // Add chrome
    chrome = new gcn::Container();
    chrome->setOpaque(false);
    chrome->setY(titlebarHeight);
    gcn::Container::add(chrome);

    // Add this window to the window container
    if (windowContainer) {
        windowContainer->add(this, modal);
    }
    else {
        throw GCN_EXCEPTION("Window::Window. no windowContainer set");
    }
}

Window::~Window()
{
    log("Window::~Window(\"%s\")", caption.c_str());

    // Free dialog bitmaps
    SDL_FreeSurface(dWindowSurface);
    //release_bitmap(dLeft);
    //release_bitmap(dMid);
    //release_bitmap(dRight);

    delete chrome;
}

void Window::setWindowContainer(WindowContainer *wc)
{
    windowContainer = wc;
}

void Window::draw(gcn::Graphics* graphics)
{

    // Resize the Window Image if it's has to be done
    if ( (dWindowSurface->w != getWidth()) || (dWindowSurface->h != getHeight()) )
    {
        SDL_FreeSurface(dWindowSurface);
        dWindowSurface = SDL_AllocSurface(SDL_SWSURFACE | SDL_SRCCOLORKEY | SDL_SRCALPHA, getWidth(), getHeight(), 
            (screen->format->BytesPerPixel*8), 0, 0, 0, 0);
        SDL_Rect sourceRect;
        sourceRect.x = sourceRect.y = 0;
        sourceRect.w = getWidth();
        sourceRect.h = getHeight();
        if ( dWindowSurface )
        { 
            // Draw the background
            dBackground->drawPattern(dWindowSurface, 0, 0, getWidth(), getHeight());
            // Draw the borders
            dBorders->draw(dWindowSurface, 0, 0, 0, 0, 4, 4); // Top-Left
            dBorders->draw(dWindowSurface, 7, 0, getWidth()-3, 0, 4, 4); // Top-Right
            dBorders->draw(dWindowSurface, 7, 15, getWidth()-3, getHeight()-3, 4, 4); // Bottom-Right
            dBorders->draw(dWindowSurface, 0, 15, 0, getHeight()-3, 4, 4); // Bottom-Left
            
            // Upper Border
            Image *dTemp = dBorders->getSubImage(4,0,1,3);
            dTemp->drawPattern(dWindowSurface, 4, 0, getWidth()-4-3, 3);
            delete dTemp;
            // Left Border
            dTemp = dBorders->getSubImage(0,4,3,1);
            dTemp->drawPattern(dWindowSurface, 0, 4, 3, getHeight()-4-3);
            delete dTemp;
            // Right Border
            dTemp = dBorders->getSubImage(8,4,3,1);
            dTemp->drawPattern(dWindowSurface, getWidth()-2, 4, 3, getHeight()-4-3);
            delete dTemp;
            // Bottom Border
            dTemp = dBorders->getSubImage(8,15,1,3);
            dTemp->drawPattern(dWindowSurface, 4, getHeight()-2, getWidth()-4-3, 3);
            delete dTemp;
    
            SDL_SetAlpha(dWindowSurface, SDL_SRCALPHA, 160);
        }
        
    }
    
    int x, y;
    getAbsolutePosition(x, y);    
    
    // Draw the window image
    SDL_Rect screenRect;
    screenRect.w = getWidth();
    screenRect.h = getHeight();
    screenRect.x = x;
    screenRect.y = y;
    if ( dWindowSurface ) SDL_BlitSurface(dWindowSurface, NULL, screen, &screenRect);

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

void Window::add(Widget *w, int x, int y)
{
    chrome->add(w, x, y);
}

void Window::mousePress(int mx, int my, int button)
{
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
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x + winWidth > screen->w) x = screen->w - winWidth;
        if (y + winHeight > screen->h) y = screen->h - winHeight;

        // Snap window to edges
        if (x < snapSize) x = 0;
        if (y < snapSize) y = 0;
        if (x + winWidth + snapSize > screen->w) x = screen->w - winWidth;
        if (y + winHeight + snapSize > screen->h) y = screen->h - winHeight;

        this->setPosition(x, y);
    }
}

void Window::mouseOut()
{
    mouseDown = false;
}

void Window::_mouseInputMessage(const gcn::MouseInput &mouseInput)
{
    if (mouseInput.getType() == gcn::MouseInput::MOTION && mouseDown) {
        // It's a window drag event
        gcn::Widget::_mouseInputMessage(mouseInput);
    }
    else {
        // It's something else
        gcn::Container::_mouseInputMessage(mouseInput);
    }
}
