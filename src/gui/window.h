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

#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <iostream>
#include <allegro.h>
#include <guichan.hpp>

class WindowContainer : public gcn::Container, public gcn::MouseListener
{
    private:
        std::string caption;	//title bar caption
        gcn::Label* captionLabel;	//TItle bar caption
        int x, y;		//x and y positions of the window
        int z;			//z position of window
        int mousePX, mousePY;	//Mouse down location relative to 0,0 of window
        int snapSize;		//Snap to window edge
        bool mouseDown;		//mouse button state
        gcn::Color titlebarColor;	//title bar color

        BITMAP *dLeft, *dMid, *dRight;

    public:
        int titlebarHeight;	//height of title bar

        //constructor
        WindowContainer(std::string text = "Window");
        ~WindowContainer();

        //draw window
        void draw(gcn::Graphics* graphics);

        //add to stop compiler complaining
        void add(Widget *w);
        //new add
        void add(Widget *w, int x, int y);

        void setDimension(const gcn::Rectangle& dimension);

        //Mouse handling
        void mousePress(int mx, int my, int button);
        void mouseRelease(int mx, int my, int button);
        void mouseMotion(int mx, int my);
        void mouseOut();
};

#endif
