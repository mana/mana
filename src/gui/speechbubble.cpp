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

#include "gui/speechbubble.h"

#include "gui/gui.h"

#include "gui/widgets/label.h"
#include "gui/widgets/textbox.h"

#include "resources/theme.h"

#include <guichan/font.hpp>

#include <guichan/widgets/label.hpp>

SpeechBubble::SpeechBubble():
    Popup("Speech", "speechbubble.xml")
{
    setMinWidth(0);
    setMinHeight(0);

    mCaption = new Label;
    mCaption->setFont(boldFont);

    mSpeechBox = new TextBox;
    mSpeechBox->setEditable(false);
    mSpeechBox->setOpaque(false);
    mSpeechBox->setTextColor(&Theme::getThemeColor(Theme::CHAT));

    add(mCaption);
    add(mSpeechBox);
}

void SpeechBubble::setCaption(const std::string &name, const gcn::Color *color)
{
    mCaption->setCaption(name);
    mCaption->adjustSize();
    mCaption->setForegroundColor(*color);
}

void SpeechBubble::setText(const std::string &text, bool showName)
{
    if (text == mText && (mCaption->getWidth() <= mSpeechBox->getMinWidth()))
        return;

    mSpeechBox->setTextColor(&Theme::getThemeColor(Theme::TEXT));

    int width = mCaption->getWidth();
    mSpeechBox->setTextWrapped(text, 130 > width ? 130 : width);
    const int speechWidth = mSpeechBox->getMinWidth();

    const int fontHeight = getFont()->getHeight();
    const int nameHeight = showName ? mCaption->getHeight() +
                           (getPadding() / 2) : 0;
    const int numRows = mSpeechBox->getNumberOfRows();
    const int height = (numRows * fontHeight) + nameHeight;

    if (width < speechWidth)
        width = speechWidth;

    setContentSize(width, height);

    mCaption->setPosition(0, 0);
    mSpeechBox->setPosition(0, nameHeight);
}
