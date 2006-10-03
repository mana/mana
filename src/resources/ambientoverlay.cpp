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

#include "ambientoverlay.h"

#include "image.h"

#include "../graphics.h"

AmbientOverlay::AmbientOverlay(Image *img, float parallax,
        float scrollX, float scrollY, float speedX, float speedY):
    mImage(img), mParallax(parallax),
    mScrollX(scrollX), mScrollY(scrollY),
    mSpeedX(speedX), mSpeedY(speedY)
{
    mImage->incRef();
}

AmbientOverlay::~AmbientOverlay()
{
    mImage->decRef();
}

void AmbientOverlay::update(int timePassed, float dx, float dy)
{
    // Self scrolling of the overlay
    mScrollX -= mSpeedX * timePassed / 10;
    mScrollY -= mSpeedY * timePassed / 10;

    // Parallax scrolling
    mScrollX += dx * mParallax;
    mScrollY += dy * mParallax;

    int imgW = mImage->getWidth();
    int imgH = mImage->getHeight();

    // Wrap values
    while (mScrollX > imgW)
        mScrollX -= imgW;
    while (mScrollX < 0)
        mScrollX += imgW;

    while (mScrollY > imgH)
        mScrollY -= imgH;
    while (mScrollY < 0)
        mScrollY += imgH;
}

void AmbientOverlay::draw(Graphics *graphics, int x, int y)
{
    graphics->drawImagePattern(mImage, (int)(-mScrollX), (int)(-mScrollY),
            x + (int)mScrollX, y + (int)mScrollY);
}
