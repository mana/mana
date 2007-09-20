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

#include "resizegrip.h"

#include <guichan/graphics.hpp>

#include "../../graphics.h"

#include "../../resources/image.h"
#include "../../resources/resourcemanager.h"

Image *ResizeGrip::gripImage = 0;
int ResizeGrip::mInstances = 0;

ResizeGrip::ResizeGrip()
{
    if (mInstances == 0)
    {
        // Load the grip image
        ResourceManager *resman = ResourceManager::getInstance();
        gripImage = resman->getImage("graphics/gui/resize.png");
    }

    mInstances++;

    setWidth(gripImage->getWidth() + 2);
    setHeight(gripImage->getHeight() + 2);
}

ResizeGrip::~ResizeGrip()
{
    mInstances--;

    if (mInstances == 0)
    {
        gripImage->decRef();
    }
}

void
ResizeGrip::draw(gcn::Graphics *graphics)
{
    static_cast<Graphics*>(graphics)->drawImage(gripImage, 0, 0);
}
