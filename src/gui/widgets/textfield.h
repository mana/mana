/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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
        TextField(const std::string &text = "", bool loseFocusOnTab = true);
        ~TextField();

        /**
         * Draws the text field.
         */
        virtual void draw(gcn::Graphics *graphics);

        /**
         * Update the alpha value to the graphic components.
         */
        void updateAlpha();

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

        /**
         * Set if the tabulator key causes auto complete
         */
         void setAutoComplete(bool b ) {mAutoComplete = b;}

         /**
         * Returns if the tabulator key causes auto complete
         */
         bool getAutoComplete() {return mAutoComplete;}

    private:
        void handlePaste();

        static int instances;
        static float mAlpha;
        static ImageRect skin;
        bool mNumeric;
        int mMinimum;
        int mMaximum;
        bool mLoseFocusOnTab;
        bool mAutoComplete;
};

#endif
