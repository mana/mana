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

#include "graphics.h"

#include "gui/gui.h"
#include "resources/image.h"
#include "resources/theme.h"

#include <guichan/graphics.hpp>

ResizeGrip::ResizeGrip()
{
    const auto gripImage = gui->getTheme()->getResizeGripImage();
    setSize(gripImage->getWidth() + 2,
            gripImage->getHeight() + 2);
}

void ResizeGrip::draw(gcn::Graphics *graphics)
{
    const auto gripImage = gui->getTheme()->getResizeGripImage();
    static_cast<Graphics*>(graphics)->drawImage(gripImage, 0, 0);
}
