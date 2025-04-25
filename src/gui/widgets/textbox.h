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

#pragma once

#include <guichan/widgets/textbox.hpp>

#include <optional>

/**
 * A text box, meant to be used inside a scroll area. Same as the Guichan text
 * box except this one doesn't have a background or border, instead completely
 * relying on the scroll area.
 *
 * \ingroup GUI
 */
class TextBox : public gcn::TextBox
{
    public:
        TextBox();

        void setTextColor(const gcn::Color *color)
        { mTextColor = color; }

        void setOutlineColor(const std::optional<gcn::Color> &color)
        { mOutlineColor = color; }

        /**
         * Sets the text after wrapping it to the current width of the widget.
         */
        void setTextWrapped(const std::string &text, int minDimension);

        /**
         * Get the minimum text width for the text box.
         */
        int getMinWidth() const { return mMinWidth; }

        /**
         * Draws the text.
         */
        void draw(gcn::Graphics *graphics) override;

    private:
        int mMinWidth;
        const gcn::Color *mTextColor;
        std::optional<gcn::Color> mOutlineColor;
};
