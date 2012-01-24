/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "gui/widgets/icon.h"

#include "graphics.h"

#include "resources/image.h"
#include "resources/resourcemanager.h"

Icon::Icon(const std::string &file)
    : mImage(0)
{
    mImage = ResourceManager::getInstance()->getImage(file);
    if (mImage)
        setSize(mImage->getWidth(), mImage->getHeight());
}

Icon::Icon(Image *image)
    : mImage(image)
{
    if (mImage)
        setSize(mImage->getWidth(), mImage->getHeight());
}

void Icon::setImage(Image *image)
{
    mImage = image;
    if (mImage)
        setSize(mImage->getWidth(), mImage->getHeight());
}

void Icon::draw(gcn::Graphics *g)
{
    if (mImage)
    {
        Graphics *graphics = static_cast<Graphics*>(g);
        const int x = (getWidth() - mImage->getWidth()) / 2;
        const int y = (getHeight() - mImage->getHeight()) / 2;
        graphics->drawImage(mImage, x, y);
    }
}
