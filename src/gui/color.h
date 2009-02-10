/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
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

#ifndef COLOUR_H
#define COLOUR_H

#include <string>
#include <vector>

#include <guichan/listmodel.hpp>

class color : public gcn::ListModel
{
    public:
        /**
         * Constructor
         */
        color();

        /**
         * Destructor
         */
        ~color();

        /**
         * Define the color replacement for a character
         *
         * @param c charater to be replaced
         * @param rgb color to replace character
         */
        void setColor(const char c, const int rgb);

        /**
         * Define the color replacement for a character
         *
         * @param c character to be replaced
         * @param r red component
         * @param g green component
         * @param b blue component
         */
        void setColor(const char c, const int r, const int g, const int b)
        {
            setColor(c, (r << 16) | (g << 8) | b);
        }

        /**
         * Return the color associated with a character, if exists
         *
         * @param c character requested
         * @param valid indicate whether character is known
         */
        int getColor(const char c, bool &valid) const;

        /**
         * Return the number of colors known
         */
        int getNumberOfElements() {return mColVector.size(); }

        /**
         * Return the name of the ith color
         *
         * @param i index of color interested in
         */
        std::string getElementAt(int i);

        /**
         * Get the color for the element at index i in the current color
         * model
         */
        int getColorAt(int i);

        /**
         * Get the character used by the color for the element at index i in
         * the current color model
         */
        char getColorCharAt(int i);

        /**
         * Set the color for the element at index i
         */
        void setColorAt(int i, int rgb);

        /**
         * Commit the colors
         */
        void commit();

        /**
         * Rollback the colors
         */
        void rollback();

    private:
        struct colorElem
        {
            colorElem(const char c, const int rgb, const std::string &text) :
                ch(c), rgb(rgb), text(text) {}
            char ch;
            int rgb;
            int committedRgb;
            std::string text;
        };
        typedef std::vector<colorElem> ColVector;
        ColVector mColVector;

        /**
         * Initialise color
         *
         * @param c character that needs initialising
         * @param rgb default color if not found in config
         * @param text identifier of color
         */
        void addColor(const char c, const int rgb, const std::string &text);
};

extern color *textcolor;

#endif
