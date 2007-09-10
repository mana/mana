/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
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

#ifndef _TMW_TRUETYPEFONT_H
#define _TMW_TRUETYPEFONT_H

#include <string>

#include <guichan/font.hpp>
#include <guichan/graphics.hpp>
#include <SDL/SDL_ttf.h>

/**
 * A wrapper around SDL_ttf for allowing the use of TrueType fonts.
 *
 * <b>NOTE:</b> This class needs SDL_ttf to be initialized.
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
        TrueTypeFont(const std::string& filename, int size);

        /**
         * Destructor.
         */
        ~TrueTypeFont();

        virtual int getWidth(const std::string& text) const;

        virtual int getHeight() const;

        /**
         * @see Font::drawString
         */
        void drawString(gcn::Graphics* graphics, const std::string& text, int x, int y);

    private:
        TTF_Font *mFont;
};

#endif
/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
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

#ifndef _TMW_TRUETYPEFONT_H
#define _TMW_TRUETYPEFONT_H

#include <string>

#include <guichan/font.hpp>
#include <guichan/graphics.hpp>
#include <SDL/SDL_ttf.h>

/**
 * A wrapper around SDL_ttf for allowing the use of TrueType fonts.
 *
 * <b>NOTE:</b> This class needs SDL_ttf to be initialized.
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
        TrueTypeFont(const std::string& filename, int size);

        /**
         * Destructor.
         */
        ~TrueTypeFont();

        virtual int getWidth(const std::string& text) const;

        virtual int getHeight() const;

        /**
         * @see Font::drawString
         */
        void drawString(gcn::Graphics* graphics, const std::string& text, int x, int y);

    private:
        TTF_Font *mFont;
};

#endif
