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

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

ImageRect ProgressBar::mBorder;
int ProgressBar::mInstances = 0;

ProgressBar::ProgressBar(float progress,
                         unsigned int width, unsigned int height,
                         Uint8 red, Uint8 green, Uint8 blue):
    gcn::Widget(),
    mProgress(progress), mRed(red), mGreen(green), mBlue(blue),
    mRedToGo(red), mGreenToGo(green), mBlueToGo(blue)
{
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
    if (mRedToGo > mRed) mRed++;
    if (mRedToGo < mRed) mRed--;
    if (mGreenToGo > mGreen) mGreen++;
    if (mGreenToGo < mGreen) mGreen--;
    if (mBlueToGo > mBlue) mBlue++;
    if (mBlueToGo < mBlue) mBlue--;
}

void
ProgressBar::draw(gcn::Graphics *graphics)
{
    static_cast<Graphics*>(graphics)->
        drawImageRect(0, 0, getWidth(), getHeight(), mBorder);

    // The bar
    if (mProgress > 0)
    {
        graphics->setColor(gcn::Color(mRed, mGreen, mBlue, 200));
        graphics->fillRectangle(gcn::Rectangle(4, 4,
                    (int)(mProgress * (getWidth() - 8)),
                    getHeight() - 8));
    }
}

void
ProgressBar::setProgress(float progress)
{
    if (progress < 0.0f) mProgress = 0.0;
    else if (progress > 1.0f) mProgress = 1.0;
    else mProgress = progress;
}

void
ProgressBar::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
    mRedToGo = red;
    mGreenToGo = green;
    mBlueToGo = blue;
}
