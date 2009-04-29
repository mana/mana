/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <guichan/widgets/textbox.hpp>

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
        /**
         * Constructor.
         */
        TextBox();

        inline void setTextColor(const gcn::Color *color)
        { mTextColor = color; }

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
        inline void draw(gcn::Graphics *graphics)
        {
            setForegroundColor(*mTextColor);
            gcn::TextBox::draw(graphics);
        }

    private:
        int mMinWidth;
        const gcn::Color *mTextColor;
};

#endif
