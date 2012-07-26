/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *  Copyright (C) 2009  Aethyra Development Team
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

#include "gui/widgets/popup.h"

#include "configuration.h"
#include "graphics.h"
#include "log.h"

#include "gui/viewport.h"

#include "gui/widgets/windowcontainer.h"

#include "resources/image.h"
#include "resources/theme.h"

#include <guichan/exception.hpp>

Popup::Popup(const std::string &name, const std::string &skin):
    mPopupName(name),
    mMinWidth(64),
    mMinHeight(25),
    mMaxWidth(graphics->getWidth()),
    mMaxHeight(graphics->getHeight())
{
    logger->log("Popup::Popup(\"%s\")", name.c_str());

    if (!windowContainer)
        throw GCN_EXCEPTION("Popup::Popup(): no windowContainer set");

    setPadding(8);

    // Loads the skin
    mSkin = Theme::instance()->load(skin);

    // Add this window to the window container
    windowContainer->add(this);

    // Popups are invisible by default
    setVisible(false);
}

Popup::~Popup()
{
    logger->log("Popup::~Popup(\"%s\")", mPopupName.c_str());

    mSkin->instances--;
}

void Popup::setWindowContainer(WindowContainer *wc)
{
    windowContainer = wc;
}

void Popup::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics*>(graphics);

    g->drawImageRect(0, 0, getWidth(), getHeight(), mSkin->getBorder());

    drawChildren(graphics);
}

gcn::Rectangle Popup::getChildrenArea()
{
    return gcn::Rectangle(getPadding(),
                          getPadding(),
                          getWidth() - getPadding() * 2,
                          getHeight() - getPadding() * 2);
}

void Popup::setContentSize(int width, int height)
{
    width += 2 * getPadding();
    height += 2 * getPadding();

    if (getMinWidth() > width)
        width = getMinWidth();
    else if (getMaxWidth() < width)
        width = getMaxWidth();
    if (getMinHeight() > height)
        height = getMinHeight();
    else if (getMaxHeight() < height)
        height = getMaxHeight();

    setSize(width, height);
}

void Popup::setLocationRelativeTo(gcn::Widget *widget)
{
    int wx, wy;
    int x, y;

    widget->getAbsolutePosition(wx, wy);
    getAbsolutePosition(x, y);

    setPosition(getX() + (wx + (widget->getWidth() - getWidth()) / 2 - x),
                getY() + (wy + (widget->getHeight() - getHeight()) / 2 - y));
}

void Popup::setMinWidth(int width)
{
    mMinWidth = width > mSkin->getMinWidth() ? width : mSkin->getMinWidth();
}

void Popup::setMinHeight(int height)
{
    mMinHeight = height > mSkin->getMinHeight() ? height : mSkin->getMinHeight();
}

void Popup::setMaxWidth(int width)
{
    mMaxWidth = width;
}

void Popup::setMaxHeight(int height)
{
    mMaxHeight = height;
}

void Popup::scheduleDelete()
{
    windowContainer->scheduleDelete(this);
}

void Popup::position(int x, int y)
{
    const int distance = 20;

    int posX = std::max(0, x - getWidth() / 2);
    int posY = y + distance;

    if (posX > graphics->getWidth() - getWidth())
        posX = graphics->getWidth() - getWidth();
    if (posY > graphics->getHeight() - getHeight())
        posY = y - getHeight() - distance;

    setPosition(posX, posY);
    setVisible(true);
    requestMoveToTop();
}

void Popup::mouseMoved(gcn::MouseEvent &event)
{
    if (viewport)
        viewport->hideBeingPopup();
}
