/*
 *  The Mana Client
 *  Copyright (c) 2009  Aethyra Development Team
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

#include "gui/widgets/label.h"

#include "textrenderer.h"

#include "resources/theme.h"

#include <guichan/exception.hpp>
#include <guichan/font.hpp>

Label::Label()
{
    setForegroundColor(Theme::getThemeColor(Theme::TEXT));
}

Label::Label(const std::string &caption) :
    gcn::Label(caption)
{
    setForegroundColor(Theme::getThemeColor(Theme::TEXT));
}

void Label::draw(gcn::Graphics *graphics)
{
    int textX;
    int textY = getHeight() / 2 - getFont()->getHeight() / 2;

    switch (getAlignment())
    {
    case Graphics::LEFT:
        textX = 0;
        break;
    case Graphics::CENTER:
        textX = getWidth() / 2;
        break;
    case Graphics::RIGHT:
        textX = getWidth();
        break;
    default:
        throw GCN_EXCEPTION("Unknown alignment.");
    }

    TextRenderer::renderText(static_cast<Graphics *>(graphics),
                             getCaption(),
                             textX,
                             textY,
                             getAlignment(),
                             getForegroundColor(),
                             getFont(),
                             mOutlineColor.has_value(),
                             mShadowColor.has_value(),
                             mOutlineColor,
                             mShadowColor);
}
