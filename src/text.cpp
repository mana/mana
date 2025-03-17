/*
 *  Support for non-overlapping floating text
 *  Copyright (C) 2008  Douglas Boffey <DougABoffey@netscape.net>
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

#include "text.h"

#include "textmanager.h"
#include "textrenderer.h"

#include "gui/gui.h"

#include "resources/theme.h"

#include <guichan/font.hpp>

int Text::mInstances = 0;

Text::Text(const std::string &text,
           int x,
           int y,
           gcn::Graphics::Alignment alignment,
           const gcn::Color *color,
           bool isSpeech,
           gcn::Font *font)
    : mText(text)
    , mColor(color)
    , mFont(font ? font : gui->getFont())
    , mIsSpeech(isSpeech)
{
    if (textManager == nullptr)
        textManager = new TextManager;

    ++mInstances;

    mHeight = mFont->getHeight();
    mWidth = mFont->getWidth(text);

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

Text::~Text()
{
    textManager->removeText(this);
    if (--mInstances == 0)
    {
        delete textManager;
        textManager = nullptr;
    }
}

void Text::setColor(const gcn::Color *color)
{
    mColor = color;
}

void Text::adviseXY(int x, int y)
{
    textManager->moveText(this, x - mXOffset, y);
}

void Text::draw(gcn::Graphics *graphics, int xOff, int yOff)
{
    if (mIsSpeech)
    {
        WidgetState state;
        state.x = mX - xOff - 5;
        state.y = mY - yOff - 5;
        state.width = mWidth + 10;
        state.height = mHeight + 10;

        auto theme = gui->getTheme();
        theme->drawSkin(static_cast<Graphics *>(graphics), SkinType::SpeechBubble, state);

        /*
        if (mWidth >= 15)
        {
            static_cast<Graphics*>(graphics)->drawImage(
                    mBubbleArrow, mX - xOff - 7 + mWidth / 2,
                    mY - yOff + mHeight + 4);
        }
        */
    }

    TextRenderer::renderText(graphics, mText,
            mX - xOff, mY - yOff, gcn::Graphics::LEFT,
            *mColor, mFont, !mIsSpeech, true);
}


void FlashText::draw(gcn::Graphics *graphics, int xOff, int yOff)
{
    if (mTimer.isSet())
    {
        if (!mTimer.passed())
        {
            if ((mTimer.elapsed() & 64) == 0)
                return;
        }
        else
        {
            mTimer.reset();
        }
    }

    Text::draw(graphics, xOff, yOff);
}
