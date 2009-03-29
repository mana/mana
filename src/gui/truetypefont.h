/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#ifndef TRUETYPEFONT_H
#define TRUETYPEFONT_H

#include <list>
#include <string>

#include <guichan/font.hpp>
#ifdef __APPLE__
#include <SDL_ttf/SDL_ttf.h>
#else
#ifdef __WIN32__
#include <SDL/SDL_ttf.h>
#else
#include <SDL_ttf.h>
#endif
#endif

class TextChunk;

/**
 * A wrapper around SDL_ttf for allowing the use of TrueType fonts.
 *
 * <b>NOTE:</b> This class initializes SDL_ttf as necessary.
 */
class TrueTypeFont : public gcn::Font
{
    public:
        /**
         * Constructor.
         *
         * @param filename  Font filename.
         * @param size      Font size.
         */
        TrueTypeFont(const std::string &filename, int size, int style = 0);

        /**
         * Destructor.
         */
        ~TrueTypeFont();

        virtual int getWidth(const std::string &text) const;

        virtual int getHeight() const;

        /**
         * @see Font::drawString
         */
        void drawString(gcn::Graphics *graphics,
                        const std::string &text,
                        int x, int y);

    private:
        TTF_Font *mFont;

        // Word surfaces cache
        std::list<TextChunk> mCache;
};

#endif
