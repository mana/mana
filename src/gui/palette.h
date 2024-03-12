/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef PALETTE_H
#define PALETTE_H

#include <guichan/color.hpp>

#include <cstdlib>
#include <string>
#include <set>
#include <vector>

// Default Gradient Delay
#define GRADIENT_DELAY 40

/**
 * Class controlling the game's color palette.
 */
class Palette
{
    public:
        /** Black Color Constant */
        static const gcn::Color BLACK;

        /** Colors can be static or can alter over time. */
        enum GradientType {
            STATIC,
            PULSE,
            SPECTRUM,
            RAINBOW
        };

        /**
         * Returns the color associated with a character, if it exists. Returns
         * Palette::BLACK if the character is not found.
         *
         * @param c character requested
         * @param valid indicate whether character is known
         *
         * @return the requested color or Palette::BLACK
         */
        const gcn::Color &getColor(char c, bool &valid);

        /**
         * Gets the color associated with the type. Sets the alpha channel
         * before returning.
         *
         * @param type the color type requested
         * @param alpha alpha channel to use
         *
         * @return the requested color
         */
        const gcn::Color &getColor(int type, int alpha = 255)
        {
            gcn::Color &col = mColors[type].color;
            col.a = alpha;
            return col;
        }

        /**
         * Gets the GradientType associated with the specified type.
         *
         * @param type the color type of the color
         *
         * @return the gradient type of the color with the given index
         */
        GradientType getGradientType(int type) const
        {
            return mColors[type].grad;
        }

        /**
         * Get the character used by the specified color.
         *
         * @param type the color type of the color
         *
         * @return the color char of the color with the given index
         */
        char getColorChar(int type) const
        {
            return mColors[type].ch;
        }

        /**
         * Gets the gradient delay for the specified type.
         *
         * @param type the color type of the color
         *
         * @return the gradient delay of the color with the given index
         */
        int getGradientDelay(int type) const
        { return mColors[type].delay; }

        /**
         * Updates all colors, that are non-static.
         */
        static void advanceGradients();

    protected:
        /** Colors used for the rainbow gradient */
        static const gcn::Color RAINBOW_COLORS[];
        static const int RAINBOW_COLOR_COUNT;

        /** Time tick, that gradient-type colors were updated the last time. */
        int mRainbowTime;

        using Palettes = std::set<Palette *>;
        static Palettes mInstances;

        Palette(int size);

        ~Palette();

        void advanceGradient();

        struct ColorElem
        {
            int type;
            gcn::Color color;
            gcn::Color testColor;
            gcn::Color committedColor;
            std::string text;
            char ch;
            GradientType grad;
            GradientType committedGrad;
            int gradientIndex;
            int delay;
            int committedDelay;

            void set(int type, gcn::Color &color, GradientType grad, int delay)
            {
                ColorElem::type = type;
                ColorElem::color = color;
                ColorElem::testColor = color;
                ColorElem::grad = grad;
                ColorElem::delay = delay;
                ColorElem::gradientIndex = rand();
            }

            int getRGB() const
            {
                return (committedColor.r << 16) | (committedColor.g << 8) |
                        committedColor.b;
            }
        };
        /** Vector containing the colors. */
        std::vector<ColorElem> mColors;
        std::vector<ColorElem*> mGradVector;
};

#endif
