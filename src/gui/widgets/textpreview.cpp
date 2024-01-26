/*
 *  The Mana Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
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

#include "gui/widgets/textpreview.h"

#include "configuration.h"
#include "textrenderer.h"

#include "gui/gui.h"
#include "gui/palette.h"
#include "gui/truetypefont.h"

#include <typeinfo>

float TextPreview::mAlpha = 1.0;

TextPreview::TextPreview(const std::string &text):
    mText(text)
{
    mTextAlpha = false;
    mFont = gui->getFont();
    mTextColor = &Theme::getThemeColor(Theme::TEXT);
    mTextBGColor = nullptr;
    mBGColor = &Theme::getThemeColor(Theme::BACKGROUND);
    mOpaque = false;
}

void TextPreview::draw(gcn::Graphics* graphics)
{
    if (config.getFloatValue("guialpha") != mAlpha)
        mAlpha = config.getFloatValue("guialpha");

    int alpha = (int) (mAlpha * 255.0f);

    if (!mTextAlpha)
        alpha = 255;

    if (mOpaque)
    {
        graphics->setColor(gcn::Color((int) mBGColor->r,
                                      (int) mBGColor->g,
                                      (int) mBGColor->b,
                                      (int)(mAlpha * 255.0f)));
        graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));
    }

    if (mTextBGColor && typeid(*mFont) == typeid(TrueTypeFont))
    {
        auto *font = static_cast<TrueTypeFont*>(mFont);
        int x = font->getWidth(mText) + 1 + 2 * ((mOutline || mShadow) ? 1 :0);
        int y = font->getHeight() + 1 + 2 * ((mOutline || mShadow) ? 1 : 0);
        graphics->setColor(gcn::Color((int) mTextBGColor->r,
                                      (int) mTextBGColor->g,
                                      (int) mTextBGColor->b,
                                      (int)(mAlpha * 255.0f)));
        graphics->fillRectangle(gcn::Rectangle(1, 1, x, y));
    }

    TextRenderer::renderText(graphics, mText, 2, 2,  gcn::Graphics::LEFT,
                             gcn::Color(mTextColor->r, mTextColor->g,
                                        mTextColor->b, alpha),
                             mFont, mOutline, mShadow);
}
