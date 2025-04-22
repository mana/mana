/*
 *  Text Renderer
 *  Copyright (C) 2009  The Mana World Development Team
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

#pragma once

#include "resources/theme.h"

#include <guichan/exception.hpp>
#include <guichan/font.hpp>

/**
 * Class for text rendering which can apply an outline and shadow.
 */
class TextRenderer
{
public:
    /**
     * Renders a specified text.
     */
    static void renderText(gcn::Graphics *graphics,
                           const std::string &text,
                           int x, int y,
                           gcn::Graphics::Alignment alignment,
                           const gcn::Color &color,
                           gcn::Font *font,
                           bool outline = false,
                           bool shadow = false,
                           const std::optional<gcn::Color> &outlineColor = {},
                           const std::optional<gcn::Color> &shadowColor = {})
    {
        switch (alignment)
        {
        case gcn::Graphics::LEFT:
            break;
        case gcn::Graphics::CENTER:
            x -= font->getWidth(text) / 2;
            break;
        case gcn::Graphics::RIGHT:
            x -= font->getWidth(text);
            break;
        default:
            throw GCN_EXCEPTION("Unknown alignment.");
        }

        // Text shadow
        if (shadow)
        {
            if (shadowColor)
            {
                graphics->setColor(*shadowColor);
            }
            else
            {
                auto sc = Theme::getThemeColor(Theme::SHADOW);
                sc.a = color.a / 2;
                graphics->setColor(sc);
            }

            if (outline)
                font->drawString(graphics, text, x + 2, y + 2);
            else
                font->drawString(graphics, text, x + 1, y + 1);
        }

        if (outline)
        {
            /*
            graphics->setColor(guiPalette->getColor(Palette::OUTLINE,
                    alpha/4));
            // TODO: Reanable when we can draw it nicely in software mode
            font->drawString(graphics, text, x + 2, y + 2);
            font->drawString(graphics, text, x + 1, y + 2);
            font->drawString(graphics, text, x + 2, y + 1);
            */

            // Text outline
            if (outlineColor)
            {
                graphics->setColor(*outlineColor);
            }
            else
            {
                auto oc = Theme::getThemeColor(Theme::OUTLINE);
                oc.a = color.a;
                graphics->setColor(oc);
            }

            font->drawString(graphics, text, x + 1, y);
            font->drawString(graphics, text, x - 1, y);
            font->drawString(graphics, text, x, y + 1);
            font->drawString(graphics, text, x, y - 1);
        }

        graphics->setColor(color);
        font->drawString(graphics, text, x, y);
    }

    /**
     * Renders a specified text.
     */
    static void renderText(gcn::Graphics *graphics,
                           const std::string &text,
                           int x,
                           int y,
                           gcn::Graphics::Alignment align,
                           gcn::Font *font,
                           const TextFormat &format)
    {
        renderText(graphics,
                   text,
                   x,
                   y,
                   align,
                   format.color,
                   font,
                   format.outlineColor.has_value(),
                   format.shadowColor.has_value(),
                   format.outlineColor,
                   format.shadowColor);
    }
};
