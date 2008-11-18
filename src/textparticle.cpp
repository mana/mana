/*
 *  The Mana World
 *  Copyright 2006 The Mana World Development Team
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

#include "graphics.h"
#include "textparticle.h"

TextParticle::TextParticle(Map *map, const std::string &text,
                           int colorR, int colorG, int colorB,
                           gcn::Font *font):
    Particle(map),
    mText(text),
    mTextFont(font),
    mColorR(colorR),
    mColorG(colorG),
    mColorB(colorB)
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
    {
        alpha *= mLifetimeLeft;
        alpha /= mFadeOut;
    }

    if (mLifetimePast < mFadeIn)
    {
        alpha *= mLifetimePast;
        alpha /= mFadeIn;
    }

    graphics->setFont(mTextFont);
    graphics->setColor(gcn::Color(mColorR, mColorG, mColorB, (int)alpha));
    graphics->drawText(mText, screenX, screenY, gcn::Graphics::CENTER);
}
