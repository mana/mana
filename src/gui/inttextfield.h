/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
         * Responds to key presses.
         */
        void
        keyPressed(gcn::KeyEvent &event);

    private:
        int mMin;            /**< Minimum value */
        int mMax;            /**< Maximum value */
        int mDefault;        /**< Default value */
        int mValue;          /**< Current value */
};

#endif
