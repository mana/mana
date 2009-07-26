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

#include "resources/ambientoverlay.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"
#include "graphics.h"

AmbientOverlay::AmbientOverlay(Image *img, float parallax,
                               float speedX, float speedY, bool keepRatio):
    mImage(img), mParallax(parallax),
    mPosX(0), mPosY(0),
    mSpeedX(speedX), mSpeedY(speedY),
    mKeepRatio(keepRatio)
{

    if (keepRatio && !mImage->isAnOpenGLOne()
        && defaultScreenWidth != 0
        && defaultScreenHeight != 0
        && graphics->getWidth() != defaultScreenWidth
        && graphics->getHeight() != defaultScreenHeight)
    {
        // Rescale the overlay to keep the ratio as if we were on
        // the default resolution...
        Image *rescaledOverlay = mImage->SDLgetScaledImage(
         (int) mImage->getWidth() / defaultScreenWidth * graphics->getWidth(),
     (int) mImage->getHeight() / defaultScreenHeight * graphics->getHeight());

        if (rescaledOverlay)
        {
            // Replace the resource with the new one...
            std::string idPath = mImage->getIdPath() + "_rescaled";
            ResourceManager::getInstance()->addResource(idPath, rescaledOverlay);
            mImage = rescaledOverlay;
        }
        else
            mImage->incRef();
    }
    else
        mImage->incRef();
}

AmbientOverlay::~AmbientOverlay()
{
    mImage->decRef();
}

void AmbientOverlay::update(int timePassed, float dx, float dy)
{
    // Self scrolling of the overlay
    mPosX -= mSpeedX * timePassed / 10;
    mPosY -= mSpeedY * timePassed / 10;

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

void AmbientOverlay::draw(Graphics *graphics, int x, int y)
{
    if (!mImage->isAnOpenGLOne() || !mKeepRatio)
        graphics->drawImagePattern(mImage,
            (int) -mPosX, (int) -mPosY, x + (int) mPosX, y + (int) mPosY);
    else
        graphics->drawRescaledImagePattern(mImage,
              (int) -mPosX, (int) -mPosY, x + (int) mPosX, y + (int) mPosY,
         (int) mImage->getWidth() / defaultScreenWidth * graphics->getWidth(),
     (int) mImage->getHeight() / defaultScreenHeight * graphics->getHeight());

}
