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

#include "joystick.h"

#include "configuration.h"
#include "log.h"

#include <cassert>

int Joystick::joystickCount = 0;

void Joystick::init()
{
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);

    // Have SDL call SDL_JoystickUpdate() automatically
    SDL_JoystickEventState(SDL_ENABLE);

    joystickCount = SDL_NumJoysticks();
    logger->log("%i joysticks/gamepads found", joystickCount);
    for (int i = 0; i < joystickCount; i++)
        logger->log("- %s", SDL_JoystickNameForIndex(i));
}

Joystick::Joystick(int no)
    : mDirection(0)
    , mUpTolerance(config.upTolerance)
    , mDownTolerance(config.downTolerance)
    , mLeftTolerance(config.leftTolerance)
    , mRightTolerance(config.rightTolerance)
    , mCalibrating(false)
    , mEnabled(config.joystickEnabled)
{
    assert(no < joystickCount);

    mJoystick = SDL_JoystickOpen(no);

    // TODO Bail out!
    if (!mJoystick)
    {
        logger->log("Couldn't open joystick: %s", SDL_GetError());
        return;
    }

    logger->log("Axes: %i ", SDL_JoystickNumAxes(mJoystick));
    logger->log("Balls: %i", SDL_JoystickNumBalls(mJoystick));
    logger->log("Hats: %i", SDL_JoystickNumHats(mJoystick));
    logger->log("Buttons: %i", SDL_JoystickNumButtons(mJoystick));
}

Joystick::~Joystick()
{
    SDL_JoystickClose(mJoystick);
}

void Joystick::update()
{
    mDirection = 0;

    // When calibrating, don't bother the outside with our state
    if (mCalibrating)
    {
        doCalibration();
        return;
    };

    if (!mEnabled)
        return;

    // X-Axis
    int position = SDL_JoystickGetAxis(mJoystick, 0);
    if (position >= mRightTolerance)
        mDirection |= RIGHT;
    else if (position <= mLeftTolerance)
        mDirection |= LEFT;

    // Y-Axis
    position = SDL_JoystickGetAxis(mJoystick, 1);
    if (position <= mUpTolerance)
        mDirection |= UP;
    else if (position >= mDownTolerance)
        mDirection |= DOWN;

    // Buttons
    for (int i = 0; i < MAX_BUTTONS; i++)
        mButtons[i] = (SDL_JoystickGetButton(mJoystick, i) == 1);
}

void Joystick::startCalibration()
{
    mUpTolerance = 0;
    mDownTolerance = 0;
    mLeftTolerance = 0;
    mRightTolerance = 0;
    mCalibrating = true;
}

void Joystick::doCalibration()
{
    // X-Axis
    int position = SDL_JoystickGetAxis(mJoystick, 0);
    if (position > mRightTolerance)
        mRightTolerance = position;
    else if (position < mLeftTolerance)
        mLeftTolerance = position;

    // Y-Axis
    position = SDL_JoystickGetAxis(mJoystick, 1);
    if (position > mDownTolerance)
        mDownTolerance = position;
    else if (position < mUpTolerance)
        mUpTolerance = position;
}

void Joystick::finishCalibration()
{
    config.leftTolerance = mLeftTolerance;
    config.rightTolerance = mRightTolerance;
    config.upTolerance = mUpTolerance;
    config.downTolerance = mDownTolerance;
    mCalibrating = false;
}

bool Joystick::buttonPressed(unsigned char no) const
{
    return (mEnabled && no < MAX_BUTTONS) ? mButtons[no] : false;
}
