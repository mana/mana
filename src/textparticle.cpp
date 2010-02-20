/*
 *  The Mana Client
 *  Copyright (C) 2006-2010  The Mana World Development Team
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

#include "textparticle.h"

#include "textrenderer.h"

#include <guichan/color.hpp>

TextParticle::TextParticle(Map *map, const std::string &text,
                           const gcn::Color *color,
                           gcn::Font *font, bool outline):
    Particle(map),
    mText(text),
    mTextFont(font),
    mColor(color),
    mOutline(outline)
{
}

void TextParticle::draw(Graphics *graphics, int offsetX, int offsetY) const
{
    if (!mAlive)
        return;

    int screenX = (int) mPos.x + offsetX;
    int screenY = (int) mPos.y - (int) mPos.z + offsetY;

    float alpha = mAlpha * 255.0f;

    if (mLifetimeLeft > -1 && mLifetimeLeft < mFadeOut)
        alpha = alpha * mLifetimeLeft / mFadeOut;

    if (mLifetimePast < mFadeIn)
        alpha = alpha * mLifetimePast / mFadeIn;

    gcn::Color color = *mColor;
    color.a = (int)alpha;

    TextRenderer::renderText(graphics, mText,
            screenX, screenY, gcn::Graphics::CENTER,
            color, mTextFont, mOutline, false);
}
