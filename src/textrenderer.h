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

#include "graphics.h"

#include "resources/theme.h"

/**
 * Class for text rendering. Used by the TextParticle, the Text and FlashText
 * objects and the Preview in the color dialog.
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
                           gcn::Graphics::Alignment align,
                           const gcn::Color &color,
                           gcn::Font *font,
                           bool outline = false,
                           bool shadow = false)
    {
        graphics->setFont(font);

        // Text shadow
        if (shadow)
        {
            graphics->setColor(Theme::getThemeColor(Theme::SHADOW,
                                                    color.a / 2));
            if (outline)
            {
                graphics->drawText(text, x + 2, y + 2, align);
            }
            else
            {
                graphics->drawText(text, x + 1, y + 1, align);
            }
        }

        if (outline) {
/*            graphics->setColor(guiPalette->getColor(Palette::OUTLINE,
                    alpha/4));
            // TODO: Reanable when we can draw it nicely in software mode
            graphics->drawText(text, x + 2, y + 2, align);
            graphics->drawText(text, x + 1, y + 2, align);
            graphics->drawText(text, x + 2, y + 1, align);*/

            // Text outline
            graphics->setColor(Theme::getThemeColor(Theme::OUTLINE, color.a));
            graphics->drawText(text, x + 1, y, align);
            graphics->drawText(text, x - 1, y, align);
            graphics->drawText(text, x, y + 1, align);
            graphics->drawText(text, x, y - 1, align);
        }

        graphics->setColor(color);
        graphics->drawText(text, x, y, align);
    }
};
