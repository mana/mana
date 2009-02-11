/*
 *  Configurable text colors
 *  Copyright (C) 2008  Douglas Boffey <dougaboffey@netscape.net>
 *
 *  This file is part of Aethyra.
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

class Colour : public gcn::ListModel
{
    public:
        /**
         * Constructor
         */
        Colour();

        /**
         * Destructor
         */
        ~Colour();

        /**
         * Define the colour replacement for a character
         *
         * @param c charater to be replaced
         * @param rgb colour to replace character
         */
        void setColour(const char c, const int rgb);

        /**
         * Define the colour replacement for a character
         *
         * @param c character to be replaced
         * @param r red component
         * @param g green component
         * @param b blue component
         */
        void setColour(const char c, const int r, const int g, const int b)
        {
            setColour(c, (r << 16) | (g << 8) | b);
        }

        /**
         * Return the colour associated with a character, if exists
         *
         * @param c character requested
         * @param valid indicate whether character is known
         */
        int getColour(const char c, bool &valid) const;

        /**
         * Return the number of colours known
         */
        int getNumberOfElements() {return mColVector.size(); }

        /**
         * Return the name of the ith colour
         *
         * @param i index of colour interested in
         */
        std::string getElementAt(int i);

        /**
         * Get the colour for the element at index i in the current colour
         * model
         */
        int getColourAt(int i);

        /**
         * Get the character used by the colour for the element at index i in
         * the current colour model
         */
        char getColourCharAt(int i);

        /**
         * Set the colour for the element at index i
         */
        void setColourAt(int i, int rgb);

        /**
         * Commit the colours
         */
        void commit();

        /**
         * Rollback the colours
         */
        void rollback();

    private:
        struct ColourElem
        {
            ColourElem(const char c, const int rgb, const std::string &text) :
                ch(c), rgb(rgb), text(text) {}
            char ch;
            int rgb;
            int committedRgb;
            std::string text;
        };
        typedef std::vector<ColourElem> ColVector;
        ColVector mColVector;

        /**
         * Initialise colour
         *
         * @param c character that needs initialising
         * @param rgb default colour if not found in config
         * @param text identifier of colour
         */
        void addColour(const char c, const int rgb, const std::string &text);
};

extern Colour *textColour;

#endif
