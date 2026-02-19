/*
 *  The Mana Client
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "gui/dragndrop.h"

#include "graphics.h"

#include "resources/image.h"
#include "resources/theme.h"
#include "utils/stringutils.h"

#include <guichan/color.hpp>

void Drag::draw(Graphics *graphics, int mouseX, int mouseY) const
{
    Item *dragItem = item.get();
    if (!dragItem)
        return;

    Image *image = dragItem->getImage();
    if (!image)
        return;

    const int x = mouseX - image->getWidth() / 2;
    const int y = mouseY - image->getHeight() / 2;

    image->setUseColor(true);
    graphics->setColor(gcn::Color(0, 0, 0, 128));
    graphics->drawImage(image, x + 2, y + 2);
    image->setUseColor(false);

    image->setAlpha(1.0f);
    graphics->drawImage(image, x, y);

    if (dragItem->getQuantity() > 1)
    {
        graphics->setColor(Theme::getThemeColor(Theme::TEXT));
        graphics->drawText(toString(dragItem->getQuantity()),
                           x + image->getWidth() / 2,
                           y + image->getHeight(),
                           gcn::Graphics::CENTER);
    }
}
