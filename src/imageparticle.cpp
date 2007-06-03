/*
 *  The Mana World
 *  Copyright 2006 The Mana World Development Team
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

#include "imageparticle.h"

#include "graphics.h"

#include "resources/image.h"

ImageParticle::ImageParticle(Map *map, Image *image):
    Particle(map),
    mImage(image)
{
}

void ImageParticle::draw(Graphics *graphics, int offsetX, int offsetY) const
{
    if (!mAlive)
        return;

    int screenX = (int) mPosX + offsetX - mImage->getWidth() / 2;
    int screenY = (int) mPosY - (int) mPosZ + offsetY - mImage->getHeight()/2;

    // Check if on screen
    if (screenX + mImage->getWidth() < 0 ||
            screenX > graphics->getWidth() ||
            screenY + mImage->getHeight() < 0 ||
            screenY > graphics->getHeight())
    {
        return;
    }

    float alphafactor = 1.0f;

    if (mLifetimeLeft > -1 && mLifetimeLeft < mFadeOut)
        alphafactor *= (float) mLifetimeLeft / (float) mFadeOut;

    if (mLifetimePast < mFadeIn)
        alphafactor *= (float) mLifetimePast / (float) mFadeIn;

    mImage->setAlpha(alphafactor);
    graphics->drawImage(mImage, screenX, screenY);
}
