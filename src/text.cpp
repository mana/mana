/*
 *  The Mana World
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

#include "text.h"

#include <cstring>

#include <guichan/font.hpp>

#include "textmanager.h"

int Text::mInstances = 0;


Text::Text(const std::string &text, int x, int y,
           gcn::Graphics::Alignment alignment, gcn::Font *font,
           gcn::Color colour) :
    mText(text), mColour(colour)
{
    if (textManager == 0)
    {
        textManager = new TextManager();
    }
    ++mInstances;
    mHeight = font->getHeight();
    mWidth = font->getWidth(text);
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
    mFont = font;
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

void Text::draw(Graphics *graphics, int xOff, int yOff)
{
    graphics->setFont(mFont);
    graphics->setColor(mColour);
    graphics->drawText(mText, mX - xOff, mY - yOff, gcn::Graphics::LEFT);
}

FlashText::FlashText(const std::string &text, int x, int y,
                     gcn::Graphics::Alignment alignment, gcn::Font *font,
                     gcn::Color colour) :
    Text(text, x, y, alignment, font, colour), mTime(0)
{
}

void FlashText::draw(Graphics *graphics, int xOff, int yOff)
{
    if (mTime)
    {
        if ((--mTime & 4) == 0)
        {
            return;
        }
    }
    Text::draw(graphics, xOff, yOff);
}
