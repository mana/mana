/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef PALETTE_H
#define PALETTE_H

#include <cstdlib>
#include <string>
#include <vector>

#include <guichan/listmodel.hpp>
#include <guichan/color.hpp>

// Generate strings from an enum ... some preprocessor fun.
#define EDEF(a) a,
#define LASTEDEF(a) a
#define ECONFIGSTR(a) Palette::getConfigName(#a),
#define LASTECONFIGSTR(a) Palette::getConfigName(#a)

#define TEXTENUM(name,def)\
    enum name { def(EDEF,LASTEDEF) };\
    static const std::string name ## Names[]
#define DEFENUMNAMES(name,def)\
    const std::string Palette::name ## Names[] = { def(ECONFIGSTR,ECONFIGSTR) "" }

// Default Gradient Delay
#define GRADIENT_DELAY 40

/**
 * Class controlling the game's color palette.
 */
class Palette : public gcn::ListModel
{
    public:
        /** List of all colors that are configurable. */
        #define COLOR_TYPE(ENTRY,LASTENTRY)\
            ENTRY(TEXT)\
            ENTRY(SHADOW)\
            ENTRY(OUTLINE)\
            ENTRY(PROGRESS_BAR)\
            ENTRY(BACKGROUND)\
            ENTRY(HIGHLIGHT)\
            ENTRY(TAB_HIGHLIGHT)\
            ENTRY(SHOP_WARNING)\
            ENTRY(ITEM_EQUIPPED)\
            ENTRY(CHAT)\
            ENTRY(GM)\
            ENTRY(PLAYER)\
            ENTRY(WHISPER)\
            ENTRY(IS)\
            ENTRY(PARTY)\
            ENTRY(SERVER)\
            ENTRY(LOGGER)\
            ENTRY(HYPERLINK)\
            ENTRY(BEING)\
            ENTRY(PC)\
            ENTRY(SELF)\
            ENTRY(GM_NAME)\
            ENTRY(NPC)\
            ENTRY(MONSTER)\
            ENTRY(UNKNOWN_ITEM)\
            ENTRY(GENERIC)\
            ENTRY(HEAD)\
            ENTRY(USABLE)\
            ENTRY(TORSO)\
            ENTRY(ONEHAND)\
            ENTRY(LEGS)\
            ENTRY(FEET)\
            ENTRY(TWOHAND)\
            ENTRY(SHIELD)\
            ENTRY(RING)\
            ENTRY(NECKLACE)\
            ENTRY(ARMS)\
            ENTRY(AMMO)\
            ENTRY(PARTICLE)\
            ENTRY(EXP_INFO)\
            ENTRY(PICKUP_INFO)\
            ENTRY(HIT_PLAYER_MONSTER)\
            ENTRY(HIT_MONSTER_PLAYER)\
            ENTRY(HIT_CRITICAL)\
            ENTRY(MISS)\
            ENTRY(HPBAR_FULL)\
            ENTRY(HPBAR_THREE_QUARTERS)\
            ENTRY(HPBAR_ONE_HALF)\
            ENTRY(HPBAR_ONE_QUARTER)\
            LASTENTRY(TYPE_COUNT)

        TEXTENUM(ColorType, COLOR_TYPE);

        /** Colors can be static or can alter over time. */
        enum GradientType {
            STATIC,
            PULSE,
            SPECTRUM,
            RAINBOW
        };

        /**
         * Constructor
         */
        Palette();

        /**
         * Destructor
         */
        ~Palette();

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
        inline const gcn::Color &getColor(ColorType type, int alpha = 255)
        {
            gcn::Color* col = &mColVector[type].color;
            col->a = alpha;
            return *col;
        }

        /**
         * Gets the committed color associated with the specified type.
         *
         * @param type the color type requested
         *
         * @return the requested committed color
         */
        inline const gcn::Color &getCommittedColor(ColorType type)
        {
            return mColVector[type].committedColor;
        }

        /**
         * Gets the test color associated with the specified type.
         *
         * @param type the color type requested
         *
         * @return the requested test color
         */
        inline const gcn::Color &getTestColor(ColorType type)
        {
            return mColVector[type].testColor;
        }

        /**
         * Sets the test color associated with the specified type.
         *
         * @param type the color type requested
         * @param color the color that should be tested
         */
        inline void setTestColor(ColorType type, gcn::Color color)
        {
            mColVector[type].testColor = color;
        }

        /**
         * Gets the GradientType associated with the specified type.
         *
         * @param type the color type of the color
         *
         * @return the gradient type of the color with the given index
         */
        inline GradientType getGradientType(ColorType type)
        {
            return mColVector[type].grad;
        }

        /**
         * Gets the gradient delay for the specified type.
         *
         * @param type the color type of the color
         *
         * @return the gradient delay of the color with the given index
         */
        inline int getGradientDelay(ColorType type)
            { return mColVector[type].delay; }

        /**
        * Get the character used by the specified color.
        *
        * @param type the color type of the color
        *
        * @return the color char of the color with the given index
        */
        inline char getColorChar(ColorType type)
        {
            return mColVector[type].ch;
        }

        /**
         * Sets the color for the specified type.
         *
         * @param type color to be set
         * @param r red component
         * @param g green component
         * @param b blue component
         */
        void setColor(ColorType type, int r, int g, int b);

        /**
         * Sets the gradient type for the specified color.
         *
         * @param grad gradient type to set
         */
        void setGradient(ColorType type, GradientType grad);

        /**
         * Sets the gradient delay for the specified color.
         *
         * @param grad gradient type to set
         */
        void setGradientDelay(ColorType type, int delay)
            { mColVector[type].delay = delay; }

        /**
         * Returns the number of colors known.
         *
         * @return the number of colors known
         */
        inline int getNumberOfElements() { return mColVector.size(); }

        /**
         * Returns the name of the ith color.
         *
         * @param i index of color interested in
         *
         * @return the name of the color
         */
        std::string getElementAt(int i);

        /**
         * Gets the ColorType used by the color for the element at index i in
         * the current color model.
         *
         * @param i the index of the color
         *
         * @return the color type of the color with the given index
         */
        ColorType getColorTypeAt(int i);

        /**
         * Commit the colors
         */
        inline void commit()
        {
            commit(false);
        }

        /**
         * Rollback the colors
         */
        void rollback();

        /**
         * Updates all colors, that are non-static.
         */
        void advanceGradient();

    private:
        /** Black Color Constant */
        static const gcn::Color BLACK;

        /** Colors used for the rainbow gradient */
        static const gcn::Color RAINBOW_COLORS[];
        static const int RAINBOW_COLOR_COUNT;
        /** Time tick, that gradient-type colors were updated the last time. */
        int mRainbowTime;

        /**
         * Define a color replacement.
         *
         * @param i the index of the color to replace
         * @param r red component
         * @param g green component
         * @param b blue component
         */
        void setColorAt(int i, int r, int g, int b);

        /**
         * Commit the colors. Commit the non-static color values, if
         * commitNonStatic is true. Only needed in the constructor.
         */
        void commit(bool commitNonStatic);

        struct ColorElem
        {
            ColorType type;
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

            void set(ColorType type, gcn::Color& color, GradientType grad,
                     const std::string &text, char c, int delay)
            {
                ColorElem::type = type;
                ColorElem::color = color;
                ColorElem::testColor = color;
                ColorElem::text = text;
                ColorElem::ch = c;
                ColorElem::grad = grad;
                ColorElem::delay = delay;
                ColorElem::gradientIndex = rand();
            }

            inline int getRGB()
            {
                return (committedColor.r << 16) | (committedColor.g << 8) |
                        committedColor.b;
            }
        };
        typedef std::vector<ColorElem> ColVector;
        /** Vector containing the colors. */
        ColVector mColVector;
        std::vector<ColorElem*> mGradVector;

        /**
         * Initialise color
         *
         * @param c character that needs initialising
         * @param rgb default color if not found in config
         * @param text identifier of color
         */
        void addColor(ColorType type, int rgb, GradientType grad,
                      const std::string &text, char c = 0, 
                      int delay = GRADIENT_DELAY);

        /**
         * Prefixes the given string with "Color", lowercases all letters but
         * the first and all following a '_'. All '_'s will be removed.
         *
         * E.g.: HIT_PLAYER_MONSTER -> HitPlayerMonster
         *
         * @param typeName string to transform
         *
         * @return the transformed string
         */
        static std::string getConfigName(const std::string &typeName);
};

extern Palette *guiPalette;

#endif
