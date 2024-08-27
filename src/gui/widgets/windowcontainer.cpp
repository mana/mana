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

#include "gui/widgets/windowcontainer.h"

#include "gui/gui.h"
#include "gui/widgets/window.h"

#include <guichan/focushandler.hpp>

WindowContainer *windowContainer = nullptr;

void WindowContainer::logic()
{
    for (auto widget : mScheduledDeletions)
        delete widget;
    mScheduledDeletions.clear();

    gcn::Container::logic();
}

void WindowContainer::draw(gcn::Graphics *graphics)
{
    gcn::Container::draw(graphics);

    if (Gui::debugDraw)
        debugDraw(graphics);
}

void WindowContainer::scheduleDelete(gcn::Widget *widget)
{
    mScheduledDeletions.insert(widget);
}

void WindowContainer::adjustAfterResize(int oldScreenWidth,
                                        int oldScreenHeight)
{
    for (auto &widget : mWidgets)
        if (auto *window = dynamic_cast<Window*>(widget))
            window->adjustPositionAfterResize(oldScreenWidth, oldScreenHeight);
}

void WindowContainer::debugDraw(gcn::Graphics *graphics)
{
    auto focusHandler = _getFocusHandler();
    auto focused = focusHandler->getFocused();
    auto modalFocused = focusHandler->getModalFocused();
    auto modalMouseFocused = focusHandler->getModalMouseInputFocused();

    for (auto widget : gcn::Widget::mWidgets)
    {
        if (!widgetIsVisible(widget))
            continue;

        int x;
        int y;
        widget->getAbsolutePosition(x, y);

        if (widget == modalMouseFocused)
        {
            graphics->setColor(gcn::Color(0, 255, 0, 128));
            graphics->fillRectangle(gcn::Rectangle(x, y,
                                                   widget->getWidth(),
                                                   widget->getHeight()));
        }

        if (widget == modalFocused)
        {
            graphics->setColor(gcn::Color(255, 0, 0, 128));
            graphics->fillRectangle(gcn::Rectangle(x, y,
                                                   widget->getWidth(),
                                                   widget->getHeight()));
        }

        if (widget == focused)
        {
            graphics->setColor(gcn::Color(255, 0, 0, 32));
            graphics->fillRectangle(gcn::Rectangle(x, y,
                                                   widget->getWidth(),
                                                   widget->getHeight()));
        }

        graphics->setColor(gcn::Color(255, 0, 0));
        graphics->drawRectangle(gcn::Rectangle(x, y,
                                               widget->getWidth(),
                                               widget->getHeight()));
    }
}

/**
 * Returns whether the widget is visible and part of the hierarchy.
 */
bool WindowContainer::widgetIsVisible(gcn::Widget *widget)
{
    if (!widget->isVisible())
        return false;

    while (widget)
    {
        if (widget == this)
            return true;

        widget = widget->getParent();
    }

    return false;
}
