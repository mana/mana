/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#pragma once

#include <string>
#include <vector>

/**
 * Class for performing a linear interpolation between colors.
 */
class DyePalette
{
    public:
        /**
         * Creates a palette based on the given string.
         * The string is either a file name or a sequence of hexadecimal RGB
         * values separated by ',' and starting with '#'.
         *
         * When withAlpha is set, the values are RGBA (used by the A dye
         * channel).
         */
        DyePalette(const std::string &description, bool withAlpha = false);

        /**
         * Gets a pixel color depending on its intensity. First color is
         * implicitly black (0, 0, 0).
         */
        void getColor(int intensity, int color[3]) const;

        /**
         * Gets a pixel color depending on its intensity.
         */
        void getColor(double intensity, int color[3]) const;

        /**
         * If the color exactly matches one of the odd-indexed colors in the
         * palette, replace it with the following color and return true.
         * Used by the S (simple) and A (simple with alpha) dye channels.
         */
        bool replaceColor(int color[4]) const;

    private:
        struct Color
        {
            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;
        };

        std::vector<Color> mColors;
        bool mWithAlpha;
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
        Dye(const std::string &dye);

        /**
         * Destroys the associated palettes.
         */
        ~Dye();

        /**
         * Modifies a pixel color. color holds red, green, blue and alpha.
         */
        void update(int color[4]) const;

        /**
         * Fills the blank in a dye placeholder with some palette names.
         */
        static void instantiate(std::string &target,
                                const std::string &palettes);

    private:

        /**
         * The order of the palettes, as well as their uppercase letter, is:
         *
         * Red, Green, Yellow, Blue, Magenta, Cyan, White (or rather gray),
         * Simple (replaces exact colors, ignoring alpha) and Alpha (like
         * Simple but including alpha).
         */
        DyePalette *mDyePalettes[9];
};
