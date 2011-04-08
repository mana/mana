/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef USER_PALETTE_H
#define USER_PALETTE_H

#include "gui/palette.h"

#include <guichan/listmodel.hpp>

/**
 * Class controlling the game's color palette.
 */
class UserPalette : public Palette, public gcn::ListModel
{
    public:
        /** List of all colors that are configurable. */
        enum {
            BEING,
            PC,
            SELF,
            GM,
            NPC,
            MONSTER,
            PARTY,
            GUILD,
            PARTICLE,
            EXP_INFO,
            PICKUP_INFO,
            HIT_PLAYER_MONSTER,
            HIT_MONSTER_PLAYER,
            HIT_CRITICAL,
            HIT_LOCAL_PLAYER_MONSTER,
            HIT_LOCAL_PLAYER_CRITICAL,
            HIT_LOCAL_PLAYER_MISS,
            MISS,
            USER_COLOR_LAST
        };

        UserPalette();

        ~UserPalette();

        /**
         * Gets the committed color associated with the specified type.
         *
         * @param type the color type requested
         *
         * @return the requested committed color
         */
        inline const gcn::Color &getCommittedColor(int type)
        {
            return mColors[type].committedColor;
        }

        /**
         * Gets the test color associated with the specified type.
         *
         * @param type the color type requested
         *
         * @return the requested test color
         */
        inline const gcn::Color &getTestColor(int type)
        {
            return mColors[type].testColor;
        }

        /**
         * Sets the test color associated with the specified type.
         *
         * @param type the color type requested
         * @param color the color that should be tested
         */
        inline void setTestColor(int type, gcn::Color color)
        {
            mColors[type].testColor = color;
        }

        /**
         * Sets the color for the specified type.
         *
         * @param type color to be set
         * @param r red component
         * @param g green component
         * @param b blue component
         */
        void setColor(int type, int r, int g, int b);

        /**
         * Sets the gradient type for the specified color.
         *
         * @param grad gradient type to set
         */
        void setGradient(int type, Palette::GradientType grad);

        /**
         * Sets the gradient delay for the specified color.
         *
         * @param grad gradient type to set
         */
        void setGradientDelay(int type, int delay)
        { mColors[type].delay = delay; }

        /**
         * Returns the number of colors known.
         *
         * @return the number of colors known
         */
        inline int getNumberOfElements() { return mColors.size(); }

        /**
         * Returns the name of the ith color.
         *
         * @param i index of color interested in
         *
         * @return the name of the color
         */
        std::string getElementAt(int i);

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
         * Gets the ColorType used by the color for the element at index i in
         * the current color model.
         *
         * @param i the index of the color
         *
         * @return the color type of the color with the given index
         */
        int getColorTypeAt(int i);

    private:
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

        /**
         * Initialise color
         *
         * @param c character that needs initialising
         * @param rgb default color if not found in config
         * @param text identifier of color
         */
        void addColor(int type, int rgb, GradientType grad,
                      const std::string &text, int delay = GRADIENT_DELAY);
};

extern UserPalette *userPalette;

#endif // USER_PALETTE_H
