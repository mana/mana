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
#include "gui/truetypefont.h"

#include <typeinfo>

TextPreview::TextPreview(const std::string &text)
    : mText(text)
{
    mFont = gui->getFont();
    mTextColor = &Theme::getThemeColor(Theme::TEXT);
}

void TextPreview::draw(gcn::Graphics* graphics)
{
    TextRenderer::renderText(graphics, mText, 2, 2,  gcn::Graphics::LEFT,
                             gcn::Color(mTextColor->r,
                                        mTextColor->g,
                                        mTextColor->b,
                                        255),
                             mFont, mOutline, mShadow);
}
