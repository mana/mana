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

#include "gui/widgets/checkbox.h"

#include "gui/gui.h"
#include "resources/theme.h"

CheckBox::CheckBox(const std::string &caption, bool selected):
    gcn::CheckBox(caption, selected)
{
}

void CheckBox::draw(gcn::Graphics* graphics)
{
    drawBox(graphics);

    graphics->setFont(getFont());
    graphics->setColor(Theme::getThemeColor(Theme::TEXT));

    const int h = getHeight() + getHeight() / 2;

    graphics->drawText(getCaption(), h - 2, 0);
}

void CheckBox::drawBox(gcn::Graphics* graphics)
{
    Theme::WidgetState state;
    state.enabled = isEnabled();
    state.hovered = mHasMouse;
    state.selected = isSelected();

    gui->getTheme()->drawCheckBox(graphics, state);
}

void CheckBox::mouseEntered(gcn::MouseEvent& event)
{
    mHasMouse = true;
}

void CheckBox::mouseExited(gcn::MouseEvent& event)
{
    mHasMouse = false;
}
