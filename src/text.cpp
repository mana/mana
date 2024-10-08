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

#include "configuration.h"
#include "textmanager.h"
#include "textrenderer.h"

#include "gui/gui.h"

#include "resources/image.h"
#include "resources/theme.h"

#include <guichan/font.hpp>

int Text::mInstances = 0;
ImageRect Text::mBubble;
Image *Text::mBubbleArrow;

Text::Text(const std::string &text, int x, int y,
           gcn::Graphics::Alignment alignment,
           const gcn::Color* color, bool isSpeech,
           gcn::Font *font) :
    mText(text),
    mColor(color),
    mIsSpeech(isSpeech)
{
    if (!font)
        mFont = gui->getFont();
    else
        mFont = font;

    if (textManager == nullptr)
    {
        textManager = new TextManager;
        Image *sbImage = Theme::getImageFromTheme("bubble.png|W:#"
            + config.getStringValue("speechBubblecolor"));
        mBubble.grid[0] = sbImage->getSubImage(0, 0, 5, 5);
        mBubble.grid[1] = sbImage->getSubImage(5, 0, 5, 5);
        mBubble.grid[2] = sbImage->getSubImage(10, 0, 5, 5);
        mBubble.grid[3] = sbImage->getSubImage(0, 5, 5, 5);
        mBubble.grid[4] = sbImage->getSubImage(5, 5, 5, 5);
        mBubble.grid[5] = sbImage->getSubImage(10, 5, 5, 5);
        mBubble.grid[6] = sbImage->getSubImage(0, 10, 5, 5);
        mBubble.grid[7] = sbImage->getSubImage(5, 10, 5, 5);
        mBubble.grid[8] = sbImage->getSubImage(10, 10, 5, 5);
        mBubbleArrow = sbImage->getSubImage(0, 15, 15, 10);
        const float bubbleAlpha = config.getFloatValue("speechBubbleAlpha");
        mBubble.setAlpha(bubbleAlpha);
        mBubbleArrow->setAlpha(bubbleAlpha);
        sbImage->decRef();
    }
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
        for (auto img : mBubble.grid)
            delete img;
        delete mBubbleArrow;
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
        static_cast<Graphics*>(graphics)->drawImageRect(
                mX - xOff - 5, mY - yOff - 5, mWidth + 10, mHeight + 10,
                mBubble);
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
