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

#include "minimapwindow.h"

#include "gui/minimap.h"

#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"

#include "resources/theme.h"

#include "map.h"

MinimapWindow::MinimapWindow():
    mMinimap(new Minimap),
    mRegionLabel(new Label),
    mWindowSkin(Theme::instance()->load("window.xml"))
{
    mRegionLabel->setAlignment(gcn::Graphics::CENTER);
    mRegionLabel->adjustSize();

    int row = 0;
    place(0, row++, mRegionLabel);
    place(0, row++, mMinimap);

    Layout &layout = getLayout();
    layout.setMargin(0);
    layout.setPadding(7);

    int w = 0;
    int h = 0;
    layout.reflow(w, h);
    setSize(w, h);
}

static gcn::Rectangle adjusted(const gcn::Rectangle &rect,
                               int left, int top, int right, int bottom)
{
    return gcn::Rectangle(rect.x + left,
                          rect.y + top,
                          rect.width - left + right,
                          rect.height - top + bottom);
}

void MinimapWindow::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics*>(graphics);

    g->drawImageRect(adjusted(mMinimap->getDimension(),
                            -5, -10 - mRegionLabel->getHeight(),
                            8, 8),
                     mWindowSkin->getBorder());

    drawChildren(graphics);
}

void MinimapWindow::setMap(Map *map)
{
    mMinimap->setMap(map);
    mRegionLabel->setCaption(map ? map->getName() : std::string());
}
