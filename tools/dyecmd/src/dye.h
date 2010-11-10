/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
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

#ifndef DYE_H
#define DYE_H

#include <vector>

#include <string>

/**
 * Class for performing a linear interpolation between colors.
 */
class Palette
{
    public:

        /**
         * Creates a palette based on the given string.
         * The string is either a file name or a sequence of hexadecimal RGB
         * values separated by ',' and starting with '#'.
         */
        Palette(const std::string &);

        /**
         * Gets a pixel color depending on its intensity.
         */
        void getColor(int intensity, int color[3]) const;

        /**
         * Tells if the palette was successfully loaded.
         */
        bool loaded() const
        { return mLoaded; }

    private:

        struct Color { unsigned char value[3]; };

        std::vector< Color > mColors;

        bool mLoaded;
};

/**
 * Class for dispatching pixel-recoloring amongst several palettes.
 */
class Dye
{
    public:

        /**
         * Creates a set of palettes based on the given string.
         *
         * The parts of string are separated by semi-colons. Each part starts
         * by an uppercase letter, followed by a colon and then a palette name.
         */
        Dye(const std::string &);

        /**
         * Destroys the associated palettes.
         */
        ~Dye();

        /**
         * Tells if the dye description was successfully loaded.
         */
        bool loaded() const
        { return mLoaded; }

        /**
         * Modifies a pixel color.
         */
        void update(int color[3]) const;

    private:

        /**
         * The order of the palettes, as well as their uppercase letter, is:
         *
         * Red, Green, Yellow, Blue, Magenta, White (or rather gray).
         */
        Palette *mPalettes[7];
        bool mLoaded;
};

#endif
