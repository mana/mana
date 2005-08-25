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

#include "progressbar.h"

#include "../graphics.h"

#include "../graphic/imagerect.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

ImageRect ProgressBar::mBorder;
int ProgressBar::mInstances = 0;

ProgressBar::ProgressBar(float progress, int x, int y,
                         unsigned int width, unsigned int height,
                         unsigned char red,
                         unsigned char green,
                         unsigned char blue):
    gcn::Widget(),
    red(red), green(green), blue(blue),
    redToGo(red), greenToGo(green), blueToGo(blue)
{
    setProgress(progress);
    setX(x);
    setY(y);
    setWidth(width);
    setHeight(height);

    if (mInstances == 0)
    {
        ResourceManager *resman = ResourceManager::getInstance();
        Image *dBorders = resman->getImage("graphics/gui/vscroll_grey.png");
        mBorder.grid[0] = dBorders->getSubImage(0, 0, 4, 4);
        mBorder.grid[1] = dBorders->getSubImage(4, 0, 3, 4);
        mBorder.grid[2] = dBorders->getSubImage(7, 0, 4, 4);
        mBorder.grid[3] = dBorders->getSubImage(0, 4, 4, 10);
        mBorder.grid[4] = resman->getImage("graphics/gui/bg_quad_dis.png");
        mBorder.grid[5] = dBorders->getSubImage(7, 4, 4, 10);
        mBorder.grid[6] = dBorders->getSubImage(0, 15, 4, 4);
        mBorder.grid[7] = dBorders->getSubImage(4, 15, 3, 4);
        mBorder.grid[8] = dBorders->getSubImage(7, 15, 4, 4);
        dBorders->decRef();
    }

    mInstances++;
}

ProgressBar::~ProgressBar()
{
    mInstances--;

    if (mInstances == 0)
    {
        delete mBorder.grid[0];
        delete mBorder.grid[1];
        delete mBorder.grid[2];
        delete mBorder.grid[3];
        mBorder.grid[4]->decRef();
        delete mBorder.grid[5];
        delete mBorder.grid[6];
        delete mBorder.grid[7];
        delete mBorder.grid[8];
    }
}

void ProgressBar::logic()
{
    // Smoothly changing the color for a nicer effect.
    if (redToGo > red) red++;
    if (redToGo < red) red--;
    if (greenToGo > green) green++;
    if (greenToGo < green) green--;
    if (blueToGo > blue) blue++;
    if (blueToGo < blue) blue--;
}

void ProgressBar::draw(gcn::Graphics *graphics)
{
    dynamic_cast<Graphics*>(graphics)->drawImageRect(0, 0, getWidth(), getHeight(),
                                         mBorder);

    // The bar
    if (progress > 0)
    {
        graphics->setColor(gcn::Color(red, green, blue, 200));
        graphics->fillRectangle(gcn::Rectangle(4, 4,
                    (int)(progress * (getWidth() - 8)), getHeight() - 8));
    }
}

void ProgressBar::setProgress(float progress)
{
    if (progress < 0.0f) this->progress = 0.0;
    else if (progress > 1.0f) this->progress = 1.0;
    else this->progress = progress;
}

float ProgressBar::getProgress()
{
    return progress;
}

void ProgressBar::setColor(unsigned char red, unsigned char green, unsigned char blue)
{
    redToGo = red;
    greenToGo = green;
    blueToGo = blue;
}

unsigned char ProgressBar::getRed()
{
    return red;
}

unsigned char ProgressBar::getGreen()
{
    return green;
}

unsigned char ProgressBar::getBlue()
{
    return blue;
}
