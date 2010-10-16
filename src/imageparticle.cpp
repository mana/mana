/*
 *  The Mana Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#include "imageparticle.h"

#include "graphics.h"

#include "resources/image.h"

ImageParticle::ImageParticle(Map *map, Image *image):
    Particle(map),
    mImage(image)
{
    if (mImage)
        mImage->incRef();
}

ImageParticle::~ImageParticle()
{
    if (mImage)
        mImage->decRef();
}

void ImageParticle::draw(Graphics *graphics, int offsetX, int offsetY) const
{
    if (!mAlive || !mImage)
        return;

    int screenX = (int) mPos.x + offsetX - mImage->getWidth() / 2;
    int screenY = (int) mPos.y - (int)mPos.z + offsetY - mImage->getHeight()/2;

    // Check if on screen
    if (screenX + mImage->getWidth() < 0 ||
            screenX > graphics->getWidth() ||
            screenY + mImage->getHeight() < 0 ||
            screenY > graphics->getHeight())
    {
        return;
    }

    mImage->setAlpha(getCurrentAlpha());
    graphics->drawImage(mImage, screenX, screenY);
}
