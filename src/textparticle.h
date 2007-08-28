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
 *
 *  $Id$
 */

#ifndef _TEXTPARTICLE_H
#define _TEXTPARTICLE_H

#include "particle.h"

#include <guichan/color.hpp>

#include "guichanfwd.h"

class TextParticle : public Particle
{
    public:
        /**
         * Constructor.
         */
        TextParticle(Map *map, const std::string &text,
                     int colorR, int colorG, int colorB,
                     gcn::Font *font);
        /**
         * Draws the particle image.
         */
        virtual void
        draw(Graphics *graphics, int offsetX, int offsetY) const;

        // hack to improve text visibility
        virtual int getPixelY() const
        { return (int) (mPos.y + mPos.z); }

    private:
        std::string mText;             /**< Text of the particle. */
        gcn::Font *mTextFont;          /**< Font used for drawing the text. */
        int mColorR, mColorG, mColorB; /**< Color used for drawing the text. */
};

#endif
