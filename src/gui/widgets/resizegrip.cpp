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

#include "gui/widgets/resizegrip.h"

#include "configuration.h"
#include "graphics.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

#include <guichan/graphics.hpp>

Image *ResizeGrip::gripImage = 0;
int ResizeGrip::mInstances = 0;
float ResizeGrip::mAlpha = 1.0;

ResizeGrip::ResizeGrip(const std::string &image)
{
    if (mInstances == 0)
    {
        // Load the grip image
        ResourceManager *resman = ResourceManager::getInstance();
        gripImage = resman->getImage(image);
        gripImage->setAlpha(mAlpha);
    }

    mInstances++;

    setWidth(gripImage->getWidth() + 2);
    setHeight(gripImage->getHeight() + 2);
}

ResizeGrip::~ResizeGrip()
{
    mInstances--;

    if (mInstances == 0)
        gripImage->decRef();
}

void ResizeGrip::draw(gcn::Graphics *graphics)
{
    if (config.getValue("guialpha", 0.8) != mAlpha)
    {
        mAlpha = config.getValue("guialpha", 0.8);
        gripImage->setAlpha(mAlpha);
    }

    static_cast<Graphics*>(graphics)->drawImage(gripImage, 0, 0);
}
