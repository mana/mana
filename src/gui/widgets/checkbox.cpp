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

#include <guichan/font.hpp>

CheckBox::CheckBox(const std::string &caption, bool selected)
    : gcn::CheckBox(caption, selected)
{
    auto &skin = gui->getTheme()->getSkin(SkinType::CheckBox);
    setWidth(skin.getMinWidth() + 2 * skin.padding + skin.spacing + getFont()->getWidth(caption));
    setHeight(skin.getMinHeight() + 2 * skin.padding);
}

void CheckBox::draw(gcn::Graphics* graphics)
{
    WidgetState widgetState(this);
    if (mHasMouse)
        widgetState.flags |= STATE_HOVERED;
    if (isSelected())
        widgetState.flags |= STATE_SELECTED;

    auto g = static_cast<Graphics *>(graphics);
    auto &skin = gui->getTheme()->getSkin(SkinType::CheckBox);
    skin.draw(g, widgetState);

    if (auto skinState = skin.getState(widgetState.flags))
    {
        auto &textFormat = skinState->textFormat;
        g->drawText(getCaption(),
                    skin.getMinWidth() + skin.padding + skin.spacing,
                    skin.padding,
                    Graphics::LEFT,
                    textFormat.bold ? boldFont : getFont(),
                    textFormat);
    }
}

void CheckBox::mouseEntered(gcn::MouseEvent& event)
{
    mHasMouse = true;
}

void CheckBox::mouseExited(gcn::MouseEvent& event)
{
    mHasMouse = false;
}
