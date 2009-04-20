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

#include "gui/speechbubble.h"

#include "gui/gui.h"

#include "gui/widgets/textbox.h"

#include "graphics.h"

#include "utils/gettext.h"

#include <guichan/font.hpp>
#include <guichan/widgets/label.hpp>

SpeechBubble::SpeechBubble():
    Popup("Speech", "graphics/gui/speechbubble.xml")
{
    setContentSize(140, 46);
    setMinWidth(29);
    setMinHeight(29);

    mCaption = new gcn::Label;
    mCaption->setFont(boldFont);

    mSpeechBox = new TextBox;
    mSpeechBox->setEditable(false);
    mSpeechBox->setOpaque(false);
    mSpeechBox->setTextColor(&guiPalette->getColor(Palette::CHAT));

    add(mCaption);
    add(mSpeechBox);

    loadPopupConfiguration();
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

    mSpeechBox->setTextColor(&guiPalette->getColor(Palette::TEXT));

    int width = mCaption->getWidth() + 2 * getPadding();
    mSpeechBox->setTextWrapped(text, 130 > width ? 130 : width);
    const int speechWidth = mSpeechBox->getMinWidth() + 2 * getPadding();

    const int fontHeight = getFont()->getHeight();
    const int nameHeight = showName ? mCaption->getHeight() + 
                           (getPadding() / 2) : 0;
    const int numRows = mSpeechBox->getNumberOfRows();
    const int height = (numRows * fontHeight) + nameHeight + getPadding();

    if (width < speechWidth)
        width = speechWidth;

    width += 2 * getPadding();

    setContentSize(width, height);

    const int xPos = ((getWidth() - width) / 2);
    const int yPos = ((getHeight() - height) / 2) + nameHeight;

    mCaption->setPosition(xPos, getPadding());
    mSpeechBox->setPosition(xPos, yPos);
}
