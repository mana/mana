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

#ifndef TEXTFIELD_H
#define TEXTFIELD_H

#include <guichan/widgets/textfield.hpp>

class ImageRect;
class TextField;

/**
 * A text field.
 *
 * \ingroup GUI
 */
class TextField : public gcn::TextField
{
    public:
        /**
         * Constructor, initializes the text field with the given string.
         */
        TextField(const std::string &text = "");

        ~TextField();

        /**
         * Draws the text field.
         */
        virtual void draw(gcn::Graphics *graphics);

        /**
         * Draws the background and border.
         */
        void drawFrame(gcn::Graphics *graphics);

        /**
         * Determine whether the field should be numeric or not
         */
        void setNumeric(bool numeric);

        /**
         * Set the range on the field if it is numeric
         */
        void setRange(int min, int max)
        {
            mMinimum = min;
            mMaximum = max;
        }

        /**
         * Processes one keypress.
         */
        void keyPressed(gcn::KeyEvent &keyEvent);

        /**
         * Set the minimum value for a range
         */
        void setMinimum(int min) { mMinimum = min; }

        /**
         * Set the maximum value for a range
         */
        void setMaximum(int max) { mMaximum = max; }

        /**
         * Return the value for a numeric field
         */
        int getValue() const;

    private:
        static int instances;
        static float mAlpha;
        static ImageRect skin;
        bool mNumeric;
        int mMinimum;
        int mMaximum;
};

#endif
