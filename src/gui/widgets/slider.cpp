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

#include "gui/widgets/slider.h"

#include "graphics.h"

#include "gui/gui.h"
#include "resources/theme.h"

Slider::Slider(double scaleEnd):
    gcn::Slider(scaleEnd)
{
    init();
}

Slider::Slider(double scaleStart, double scaleEnd):
    gcn::Slider(scaleStart, scaleEnd)
{
    init();
}

void Slider::init()
{
    setFrameSize(0);
    setMarkerLength(gui->getTheme()->getMinWidth(SkinType::SliderHandle));
}

void Slider::draw(gcn::Graphics *graphics)
{
    WidgetState state(this);
    if (mHasMouse)
        state.flags |= STATE_HOVERED;

    auto theme = gui->getTheme();
    theme->drawSkin(static_cast<Graphics*>(graphics), SkinType::Slider, state);

    WidgetState handleState(state);
    handleState.x += getMarkerPosition();
    theme->drawSkin(static_cast<Graphics*>(graphics), SkinType::SliderHandle, handleState);
}

void Slider::drawMarker(gcn::Graphics *graphics)
{
    // Marker is drawn in Slider::draw
}

void Slider::mouseEntered(gcn::MouseEvent& event)
{
    mHasMouse = true;
}

void Slider::mouseExited(gcn::MouseEvent& event)
{
    mHasMouse = false;
}
