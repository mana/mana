/*
 *  Support for non-overlapping floating text
 *  Copyright (C) 2008  Douglas Boffey <DougABoffey@netscape.net>
 *  Copyright (C) 2008  The Mana World Development Team
 *
 *  This file is part of Aethyra.
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

#include <guichan/font.hpp>

#include "text.h"
#include "textmanager.h"

#include "gui/gui.h"
#include "gui/palette.h"
#include "gui/textrenderer.h"

int Text::mInstances = 0;

Text::Text(const std::string &text, int x, int y,
           gcn::Graphics::Alignment alignment, const gcn::Color* color) :
    mText(text),
    mColor(color)
{
    if (textManager == 0)
    {
        textManager = new TextManager();
    }
    ++mInstances;
    mHeight = boldFont->getHeight();
    mWidth = boldFont->getWidth(text);
    switch (alignment)
    {
        case gcn::Graphics::LEFT:
            mXOffset = 0;
            break;
        case gcn::Graphics::CENTER:
            mXOffset = mWidth / 2;
            break;
        case gcn::Graphics::RIGHT:
            mXOffset = mWidth;
            break;
    }
    mX = x - mXOffset;
    mY = y;
    textManager->addText(this);
}

void Text::adviseXY(int x, int y)
{
    textManager->moveText(this, x - mXOffset, y);
}

Text::~Text()
{
    textManager->removeText(this);
    if (--mInstances == 0)
    {
        delete textManager;
        textManager = 0;
    }
}

void Text::draw(gcn::Graphics *graphics, int xOff, int yOff)
{
    graphics->setFont(boldFont);

    TextRenderer::renderText(graphics, mText,
            mX - xOff, mY - yOff, gcn::Graphics::LEFT,
            *mColor, boldFont, true);
}

FlashText::FlashText(const std::string &text, int x, int y,
                     gcn::Graphics::Alignment alignment,
                     const gcn::Color *color) :
    Text(text, x, y, alignment, color),
    mTime(0)
{
}

void FlashText::draw(gcn::Graphics *graphics, int xOff, int yOff)
{
    if (mTime)
    {
        if ((--mTime & 4) == 0)
            return;
    }
    Text::draw(graphics, xOff, yOff);
}
