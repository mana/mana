/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#ifndef INTTEXTFIELD_H
#define INTTEXTFIELD_H

#include "textfield.h"

/**
 * TextBox which only accepts numbers as input.
 */
class IntTextField : public TextField
{
    public:
        /**
         * Constructor, sets default value.
         */
        IntTextField(int def = 0);

        /**
         * Sets the minimum and maximum values of the text box.
         */
        void setRange(int minimum, int maximum);

        /**
         * Returns the value in the text box.
         */
        int getValue();

        /**
         * Reset the field to the default value.
         */
        void reset();

        /**
         * Set the value of the text box to the specified value.
         */
        void setValue(int value);

        /**
         * Set the default value of the text box to the specified value.
         */
        void setDefaultValue(int value);

        /**
         * Responds to key presses.
         */
        void keyPressed(gcn::KeyEvent &event);

    private:
        int mMin;            /**< Minimum value */
        int mMax;            /**< Maximum value */
        int mDefault;        /**< Default value */
        int mValue;          /**< Current value */
};

#endif
