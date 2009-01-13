/*
 *  The Mana World
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008  The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "speechbubble.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"

SpeechBubble::SpeechBubble()
{
    mSpeechBox = new TextBox();
    mSpeechBox->setEditable(false);
    mSpeechBox->setOpaque(false);

    mSpeechArea = new ScrollArea(mSpeechBox);

    // Height == Top Graphic (14px) + 1 Row of Text (15px) + Bottom Graphic (17px)
    setContentSize(135, 46);
    setTitleBarHeight(0);
    loadSkin("graphics/gui/speechbubble.xml");

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

void SpeechBubble::setText(const std::string mText)
{
    mSpeechBox->setTextWrapped( mText );

    int numRows = mSpeechBox->getNumberOfRows();

    // 31 == speechbubble Top + Bottom graphic pixel heights
    // 15 == height of each line of text (based on font heights)
    setContentSize(135, 31 + (numRows * 15) );
    mSpeechArea->setDimension(gcn::Rectangle(4, 15, 130, (31 + (numRows * 14)) - 18 ));
}

unsigned int SpeechBubble::getNumRows()
{
    return mSpeechBox->getNumberOfRows();
}
