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

#include "window.h"
#include "gui.h"
#include <guichan/allegro.hpp>

WindowContainer::WindowContainer(std::string text) :
    caption(text),
    mousePX(0),
    mousePY(0),
    snapSize(8),
    mouseDown(false),
    titlebarHeight(48)
{
    titlebarColor.r = 32;
    titlebarColor.g = 32;
    titlebarColor.b = 128;

    gcn::Widget::setBaseColor(gcn::Color(255, 255, 255));

    /* Crappy window caption
       captionLabel = new Label(caption.c_str());
       captionLabel->setPosition(3, 3);
       add(captionLabel);
       */

    //load dialog title bar image
    dLeft = load_bitmap("skin/dialogLeft.bmp", NULL);
    dMid = load_bitmap("skin/dialogMiddle.bmp", NULL);
    dRight = load_bitmap("skin/dialogRight.bmp", NULL);

    //register mouse listener
    addMouseListener(this);
}

WindowContainer::~WindowContainer()
{
    //delete captionLabel;

    //free dialog bitmaps
    release_bitmap(dLeft);
    release_bitmap(dMid);
    release_bitmap(dRight);
}

void WindowContainer::draw(gcn::Graphics* graphics)
{
    //draw container background
    //Container::draw(graphics);
    if (mOpaque)
    {
        graphics->setColor(getBaseColor());
        graphics->fillRectangle(gcn::Rectangle(0, 24,
                    getDimension().width, getDimension().height));
    }


    //skinned dialog render
    if (typeid(*graphics) == typeid(gcn::AllegroGraphics))
    {
        //its allegro !!
        gcn::AllegroGraphics *gfx = (gcn::AllegroGraphics*)graphics;	//woo
        BITMAP *screen = gfx->getTarget();			//get screen surface

        //left
        masked_blit(dLeft, screen, 0, 0, x, y, 24, 24);
        //center
        for (int i = 1; i <= (getDimension().width - 24) / 24; i++)
        {
            blit(dMid, screen, 0, 0, x + i * 24, y, 24, 24);
        }
        //right
        masked_blit(dRight, screen, 0, 0, x + getDimension().width - 24, y, 24, 24);

        //draw caption
        int rtm = alfont_text_mode(-1);
        gui_text(gui_bitmap, caption.c_str(), x + 4, y + 4, gui_skin.button.textcolor[0], FALSE);
        alfont_text_mode(rtm);
    } else
    {
        //plain title bar
        graphics->setColor(titlebarColor);
        graphics->fillRectangle(gcn::Rectangle(0, 0,
                    getDimension().width, titlebarHeight));
    }


    drawChildren(graphics);
}

//set dimension
void WindowContainer::setDimension(const gcn::Rectangle &dimension)
{
    gcn::Widget::setDimension(gcn::Rectangle(dimension.x, dimension.y,
                dimension.width, dimension.height + titlebarHeight));
}

//adding new widget
void WindowContainer::add(gcn::Widget *w)
{
    w->setPosition(w->getDimension().x, w->getDimension().y + titlebarHeight);
    gcn::Container::add(w);
}
void WindowContainer::add(Widget *w, int x, int y)
{
    w->setPosition(x, y + titlebarHeight);
    gcn::Container::add(w);
}

void WindowContainer::mousePress(int mx, int my, int button)
{
    x = this->getDimension().x;
    y = this->getDimension().y;

    mouseDown = true;

    mousePX = mx;
    mousePY = my;
}

void WindowContainer::mouseRelease(int mx, int my, int button)
{
    mouseDown = false;
}

//window move
void WindowContainer::mouseMotion(int mx, int my)
{
    if (mouseDown)
    {
        int winWidth = this->getDimension().width;
        int winHeight = this->getDimension().height;
        x = this->getDimension().x;
        y = this->getDimension().y;

        x = x - (mousePX - mx);
        y = y - (mousePY - my);

        //keep guichan window inside window
        if (x < 0)
            x = 0;
        if (y < 0)
            y = 0;
        if (x + winWidth > 799)
            x = 799 - winWidth;
        if (y + winWidth > 599)
            y = 599 - winHeight;

        //snap window to edges
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

void WindowContainer::mouseOut()
{
    mouseDown = false;
}
