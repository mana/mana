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
 *
 *  $Id$
 */

#ifndef _TMW_JOYSTICK_H
#define _TMW_JOYSTICK_H

#include <SDL.h>

class Joystick
{
    public:
        /**
         * Number of buttons we can handle
         */
        static const unsigned char MAX_BUTTONS = 6;

        /**
         * Directions, to be used as bitmask values
         */
        static const char UP = 1;
        static const char DOWN = 2;
        static const char LEFT = 4;
        static const char RIGHT = 8;

        /**
         * Initializes the joystick subsystem
         */
        static void init();

        /**
         * Returns the number of available joysticks
         */
        static int getNumberOfJoysticks() { return joystickCount; }

        /**
         * Constructor, pass the number of the joystick the new object
         * should access.
         */
        Joystick(int no);

        ~Joystick();

        /**
         * Updates the direction and button information.
         */
        void update();

        void startCalibration();
        void finishCalibration();
        bool isCalibrating() { return mCalibrating; };

        bool buttonPressed(unsigned char no);

        bool isUp() { return mDirection & UP; };
        bool isDown() { return mDirection & DOWN; };
        bool isLeft() { return mDirection & LEFT; };
        bool isRight() { return mDirection & RIGHT; };

    protected:
        unsigned char mDirection;
        bool mButtons[MAX_BUTTONS];
        SDL_Joystick *mJoystick;

        int mUpTolerance, mDownTolerance, mLeftTolerance, mRightTolerance;
        bool mCalibrating;

        static int joystickCount;

        void doCalibration();
};

#endif
