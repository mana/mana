/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gui/widgets/slider.h"

#include "configuration.h"
#include "graphics.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

Image *Slider::hStart, *Slider::hMid, *Slider::hEnd, *Slider::hGrip;
Image *Slider::vStart, *Slider::vMid, *Slider::vEnd, *Slider::vGrip;
float Slider::mAlpha = 1.0;
int Slider::mInstances = 0;

Slider::Slider(double scaleEnd):
    gcn::Slider(scaleEnd)
{
    init();
}

Slider::Slider(double scaleStart, double scaleEnd):
    gcn::Slider(scaleStart, scaleEnd)
{
    init();
}

Slider::~Slider()
{
    mInstances--;

    if (mInstances == 0)
    {
        delete hStart;
        delete hMid;
        delete hEnd;
        delete hGrip;
        delete vStart;
        delete vMid;
        delete vEnd;
        delete vGrip;
    }
}

void Slider::init()
{
    int x, y, w, h,o1,o2;
    setFrameSize(0);

    // Load resources
    if (mInstances == 0)
    {
        ResourceManager *resman = ResourceManager::getInstance();
        Image *slider = resman->getImage("graphics/gui/slider.png");

        x = 0; y = 0;
        w = 15; h = 6;
        o1 = 4; o2 = 11;
        hStart = slider->getSubImage(x, y, o1 - x, h);
        hMid = slider->getSubImage(o1, y, o2 - o1, h);
        hEnd = slider->getSubImage(o2, y, w - o2 + x, h);

        x = 6; y = 8;
        w = 9; h = 10;
        hGrip = slider->getSubImage(x, y, w, h);

        x = 0; y = 6;
        w = 6; h = 21;
        o1 = 10; o2 = 18;
        vStart = slider->getSubImage(x, y, w, o1 - y);
        vMid = slider->getSubImage(x, o1, w, o2 - o1);
        vEnd = slider->getSubImage(x, o2, w, h - o2 + y);

        x = 6; y = 8;
        w = 9; h = 10;
        vGrip = slider->getSubImage(x, y, w, h);

        slider->decRef();

        hStart->setAlpha(mAlpha);
        hMid->setAlpha(mAlpha);
        hEnd->setAlpha(mAlpha);
        hGrip->setAlpha(mAlpha);

        vStart->setAlpha(mAlpha);
        vMid->setAlpha(mAlpha);
        vEnd->setAlpha(mAlpha);
        vGrip->setAlpha(mAlpha);
    }

    mInstances++;

    setMarkerLength(hGrip->getWidth());
}

void Slider::draw(gcn::Graphics *graphics)
{
    int w = getWidth();
    int h = getHeight();
    int x = 0;
    int y = (h - hStart->getHeight()) / 2;

    if (config.getValue("guialpha", 0.8) != mAlpha)
    {
        mAlpha = config.getValue("guialpha", 0.8);
        hStart->setAlpha(mAlpha);
        hMid->setAlpha(mAlpha);
        hEnd->setAlpha(mAlpha);
        hGrip->setAlpha(mAlpha);

        vStart->setAlpha(mAlpha);
        vMid->setAlpha(mAlpha);
        vEnd->setAlpha(mAlpha);
        vGrip->setAlpha(mAlpha);
    }

    static_cast<Graphics*>(graphics)->drawImage(hStart, x, y);

    w -= hStart->getWidth() + hEnd->getWidth();
    x += hStart->getWidth();

    static_cast<Graphics*>(graphics)->
        drawImagePattern(hMid, x, y, w, hMid->getHeight());

    x += w;
    static_cast<Graphics*>(graphics)->drawImage(hEnd, x, y);

    drawMarker(graphics);
}

void Slider::drawMarker(gcn::Graphics *graphics)
{
    static_cast<Graphics*>(graphics)->
       drawImage(hGrip, getMarkerPosition(), (getHeight() - hGrip->getHeight()) / 2);
}
