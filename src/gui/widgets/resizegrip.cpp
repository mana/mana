/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "gui/widgets/resizegrip.h"

#include "configuration.h"
#include "graphics.h"

#include "resources/image.h"
#include "resources/theme.h"

#include <guichan/graphics.hpp>

Image *ResizeGrip::gripImage = 0;
int ResizeGrip::mInstances = 0;
float ResizeGrip::mAlpha = 1.0;

ResizeGrip::ResizeGrip(const std::string &image)
{
    if (mInstances == 0)
    {
        // Load the grip image
        gripImage = Theme::getImageFromTheme(image);
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
    //if (config.getFloatValue("guialpha") != mAlpha)
    //{
    //    mAlpha = config.getFloatValue("guialpha");
    //    gripImage->setAlpha(mAlpha);
    //}

    //static_cast<Graphics*>(graphics)->drawImage(gripImage, 0, 0);
}
