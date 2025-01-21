/*
 *  Speech bubbles
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "gui/widgets/popup.h"

#include "resources/theme.h"

class TextBox;

class SpeechBubble : public Popup
{
    public:
        SpeechBubble();

        /**
         * Sets the name displayed for the speech bubble, and in what color.
         */
        void setCaption(const std::string &name,
                        const gcn::Color *color =
                        &Theme::getThemeColor(Theme::TEXT));

        /**
         * Sets the text to be displayed.
         */
        void setText(const std::string &text, bool showName = true);

    private:
        std::string mText;
        gcn::Label *mCaption;
        TextBox *mSpeechBox;
};
