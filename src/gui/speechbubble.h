/*
 *  Speech bubbles
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008  The Mana World Development Team
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

#ifndef SPEECHBUBBLE_H
#define SPEECHBUBBLE_H

#include "gui/palette.h"

#include "gui/widgets/popup.h"

class TextBox;

class SpeechBubble : public Popup
{
    public:
        /**
         * Constructor. Initializes the speech bubble.
         */
        SpeechBubble();

        /**
         * Sets the name displayed for the speech bubble, and in what color.
         */
        void setCaption(const std::string &name,
                        const gcn::Color *color =
                        &guiPalette->getColor(Palette::TEXT));

        /**
         * Sets the text to be displayed.
         */
        void setText(const std::string &text, bool showName = true);

        /**
         * Sets the location in which the speech bubble will be displayed.
         */
        void setLocation(int x, int y);

    private:
        std::string mText;
        gcn::Label *mCaption;
        TextBox *mSpeechBox;
};

#endif
