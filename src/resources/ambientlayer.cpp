/*
 *  The Mana Client
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

#include "resources/ambientlayer.h"

#include "graphics.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

AmbientLayer::AmbientLayer(Image *img)
    : mImage(img)
{}

AmbientLayer::~AmbientLayer() = default;

void AmbientLayer::update(int timePassed, float dx, float dy)
{
    // Self scrolling of the overlay
    mPosX -= mSpeedX * timePassed;
    mPosY -= mSpeedY * timePassed;

    // Parallax scrolling
    mPosX += dx * mParallax;
    mPosY += dy * mParallax;

    int imgW = mImage->getWidth();
    int imgH = mImage->getHeight();

    // Wrap values
    while (mPosX > imgW)
        mPosX -= imgW;
    while (mPosX < 0)
        mPosX += imgW;

    while (mPosY > imgH)
        mPosY -= imgH;
    while (mPosY < 0)
        mPosY += imgH;
}

void AmbientLayer::draw(Graphics *graphics)
{
    const auto screenWidth = graphics->getWidth();
    const auto screenHeight = graphics->getHeight();
    const auto x = static_cast<int>(mPosX);
    const auto y = static_cast<int>(mPosY);

    if (!mKeepRatio)
    {
        graphics->drawImagePattern(mImage,
                                   -x, -y,
                                   screenWidth + x,
                                   screenHeight + y);
    }
    else
    {
        graphics->drawRescaledImagePattern(mImage,
                                           -x, -y,
                                           screenWidth + x,
                                           screenHeight + y,
                                           mImage->getWidth() * screenWidth / 800,
                                           mImage->getHeight() * screenHeight / 600);
    }
}
