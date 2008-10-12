/*
 *  The Legend of Mazzeroth
 *  Copyright (C) 2008, The Legend of Mazzeroth Development Team
 *
 *  This file is part of The Legend of Mazzeroth based on original code
 *  from The Mana World.
 *
 *  The Legend of Mazzeroth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Legend of Mazzeroth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Legend of Mazzeroth; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include <guichan/font.hpp>

#include "speechbubble.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

// TODO: Fix windows so that they can each load their own skins without the
// other windows overriding another window's skin.
SpeechBubble::SpeechBubble()//:
//    Window("Message", false, NULL, "graphics/gui/speechbubble.xml")
{
    mSpeechBox = new TextBox();
    mSpeechBox->setEditable(false);
    mSpeechBox->setOpaque(false);

    mSpeechArea = new ScrollArea(mSpeechBox);

    // Height == Top Graphic (14px) + 1 Row of Text (15px) + Bottom Graphic (17px)
    setContentSize(140, 46);
    setTitleBarHeight(5);

    mSpeechArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mSpeechArea->setVerticalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mSpeechArea->setDimension(gcn::Rectangle(4, 15, 130, 28));
    mSpeechArea->setOpaque(false);

    add(mSpeechArea);

    setLocationRelativeTo(getParent());

    // LEEOR / TODO: This causes an exception error.
    //moveToBottom(getParent());

    mSpeechBox->setTextWrapped( "" );
}

void SpeechBubble::setText(std::string mText)
{
    while (mText[0] == ' ')
    {
        mText = mText.substr(1, mText.size());
    }

    mSpeechBox->setMinWidth(140);
    mSpeechBox->setTextWrapped( mText );

    int numRows = mSpeechBox->getNumberOfRows();

    if (numRows > 1)
    {
        // 15 == height of each line of text (based on font heights)
        // 14 == speechbubble Top + Bottom graphic pixel heights
        setContentSize(mSpeechBox->getMinWidth() + 15, 15 + (numRows * 15));
        mSpeechArea->setDimension(gcn::Rectangle(4, 15, mSpeechBox->getMinWidth() + 5, 
                                  3 +(numRows * 14)));
    }
    else
    {
        int width = getFont()->getWidth(this->getCaption());
        if (width < getFont()->getWidth(mText))
            width = getFont()->getWidth(mText);
        setContentSize(width + 15, 30);
        mSpeechArea->setDimension(gcn::Rectangle(4, 15, width + 5, 17));
    }
}

unsigned int SpeechBubble::getNumRows()
{
    return mSpeechBox->getNumberOfRows();
}
