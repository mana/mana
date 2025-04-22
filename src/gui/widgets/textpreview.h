/*
 *  The Mana Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
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

#include <guichan/color.hpp>
#include <guichan/font.hpp>
#include <guichan/widget.hpp>

/**
 * Preview widget for particle colors, etc.
 */
class TextPreview : public gcn::Widget
{
    public:
        TextPreview(const std::string &text);

        /**
         * Sets the color the text is printed in.
         *
         * @param color the color to set
         */
        void setTextColor(const gcn::Color *color)
        {
            mTextColor = color;
        }

        /**
         * Sets the font to render the text in.
         *
         * @param font the font to use.
         */
        void setFont(gcn::Font *font)
        {
            mFont = font;
        }

        /**
         * Sets whether to use a shadow while rendering.
         *
         * @param shadow true, if a shadow is wanted, false else
         */
        void setShadow(bool shadow)
        {
            mShadow = shadow;
        }

        /**
         * Sets whether to use an outline while rendering.
         *
         * @param outline true, if an outline is wanted, false else
         */
        void setOutline(bool outline)
        {
            mOutline = outline;
        }

        /**
         * Widget's draw method. Does the actual job.
         *
         * @param graphics graphics to draw into
         */
        void draw(gcn::Graphics *graphics) override;

    private:
        gcn::Font *mFont;
        std::string mText;
        const gcn::Color *mTextColor;
        bool mShadow = false;
        bool mOutline = false;
};
