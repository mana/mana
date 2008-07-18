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
 *  $Id: joystick.h 3587 2007-09-20 13:24:20Z b_lindeijer $
 */

#ifndef _TMW_JOYSTICK_H
#define _TMW_JOYSTICK_H

#include <SDL.h>

class Joystick
{
    public:
        /**
         * Number of buttons we can handle.
         */
        enum { MAX_BUTTONS = 6 };

        /**
         * Directions, to be used as bitmask values.
         */
        enum { UP = 1, DOWN = 2, LEFT = 4, RIGHT = 8 };

        /**
         * Initializes the joystick subsystem.
         */
        static void init();

        /**
         * Returns the number of available joysticks.
         */
        static int getNumberOfJoysticks() { return joystickCount; }

        /**
         * Constructor, pass the number of the joystick the new object
         * should access.
         */
        Joystick(int no);

        ~Joystick();

        bool
        isEnabled() const { return mEnabled; }

        void
        setEnabled(bool enabled) { mEnabled = enabled; }

        /**
         * Updates the direction and button information.
         */
        void
        update();

        void
        startCalibration();

        void
        finishCalibration();

        bool
        isCalibrating() const { return mCalibrating; }

        bool buttonPressed(unsigned char no) const;

        bool isUp() const { return mDirection & UP; };
        bool isDown() const { return mDirection & DOWN; };
        bool isLeft() const { return mDirection & LEFT; };
        bool isRight() const { return mDirection & RIGHT; };

    protected:
        unsigned char mDirection;
        bool mButtons[MAX_BUTTONS];
        SDL_Joystick *mJoystick;

        int mUpTolerance, mDownTolerance, mLeftTolerance, mRightTolerance;
        bool mCalibrating;
        bool mEnabled;

        static int joystickCount;

        void doCalibration();
};

#endif
