/*
 *  The Mana World
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "gui/textpopup.h"

#include "gui/gui.h"
#include "gui/palette.h"

#include "gui/widgets/label.h"

#include "graphics.h"
#include "units.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <guichan/font.hpp>
#include <guichan/widgets/label.hpp>

TextPopup::TextPopup():
    Popup("TextPopup")
{
    const int fontHeight = getFont()->getHeight();

    mText1 = new Label;
    mText1->setPosition(0, 0);

    mText2 = new Label;
    mText2->setPosition(0, fontHeight);

    add(mText1);
    add(mText2);
    addMouseListener(this);
}

TextPopup::~TextPopup()
{
}

void TextPopup::show(int x, int y, const std::string &str1, const std::string &str2)
{
    mText1->setCaption(str1);
    mText1->adjustSize();
    mText2->setCaption(str2);
    mText2->adjustSize();

    int minWidth = mText1->getWidth();
    if (mText2->getWidth() > minWidth)
        minWidth = mText2->getWidth();

    if (!str2.empty())
        setContentSize(minWidth, mText1->getFont()->getHeight() * 2);
    else
        setContentSize(minWidth, mText1->getFont()->getHeight());

    const int distance = 20;

    int posX = std::max(0, x - getWidth() / 2);
    int posY = y + distance;

    if (posX + getWidth() > graphics->getWidth())
        posX = graphics->getWidth() - getWidth();
    if (posY + getHeight() > graphics->getHeight())
        posY = y - getHeight() - distance;

    setPosition(posX, posY);
    setVisible(true);
    requestMoveToTop();
}

void TextPopup::mouseMoved(gcn::MouseEvent &event)
{
    Popup::mouseMoved(event);

    // When the mouse moved on top of the popup, hide it
    setVisible(false);
}
