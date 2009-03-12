/*
 *  The Mana World
 *  Copyright (C) 2006  The Mana World Development Team
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

#include "textpreview.h"

#include "../gui.h"
#include "../palette.h"
#include "../textrenderer.h"
#include "../truetypefont.h"

TextPreview::TextPreview(const std::string* text)
{
    mText = text;
    mFont = gui->getFont();
    mTextColor = &guiPalette->getColor(Palette::TEXT);
    mTextBGColor = NULL;
    mBGColor = &guiPalette->getColor(Palette::BACKGROUND);
}

void TextPreview::draw(gcn::Graphics* graphics)
{
    graphics->setColor(*mBGColor);
    graphics->fillRectangle(gcn::Rectangle(0, 0, getWidth(), getHeight()));

    if (mTextBGColor && typeid(*mFont) == typeid(TrueTypeFont)) {
        TrueTypeFont *font = static_cast<TrueTypeFont*>(mFont);
        graphics->setColor(*mTextBGColor);
        int x = font->getWidth(*mText) + 1 + 2 * ((mOutline || mShadow) ? 1 :0);
        int y = font->getHeight() + 1 + 2 * ((mOutline || mShadow) ? 1 : 0);
        graphics->fillRectangle(gcn::Rectangle(1, 1, x, y));
    }

    TextRenderer::renderText(graphics, *mText, 2, 2,  gcn::Graphics::LEFT,
            mTextColor, mFont, mOutline, mShadow);
}
