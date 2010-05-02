/*
 *  The Mana Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef TEXTPARTICLE_H
#define TEXTPARTICLE_H

#include "guichanfwd.h"
#include "particle.h"

class TextParticle : public Particle
{
    public:
        /**
         * Constructor.
         */
        TextParticle(Map *map, const std::string &text,
                     const gcn::Color* color,
                     gcn::Font *font, bool outline = false);

        /**
         * Draws the particle image.
         */
        virtual bool draw(Graphics *graphics, int offsetX, int offsetY) const;

        // hack to improve text visibility
        virtual int getPixelY() const
        { return (int) (mPos.y + mPos.z); }

    private:
        std::string mText;             /**< Text of the particle. */
        gcn::Font *mTextFont;          /**< Font used for drawing the text. */
        const gcn::Color *mColor;      /**< Color used for drawing the text. */
        bool mOutline;                 /**< Make the text better readable */
};

#endif
