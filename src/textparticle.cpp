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

#include "textparticle.h"

#include "graphics.h"

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

bool TextParticle::draw(Graphics *graphics, int offsetX, int offsetY) const
{
    if (!isAlive())
        return false;

    int screenX = (int) mPos.x + offsetX;
    int screenY = (int) mPos.y - (int) mPos.z + offsetY;

    gcn::Color color = *mColor;
    color.a = getCurrentAlpha() * 255;

    graphics->drawText(mText,
            screenX, screenY, gcn::Graphics::CENTER,
            color, mTextFont, mOutline, false);

    return true;
}
