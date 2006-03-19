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

#include "setup_joystick.h"

#include <guichan/widgets/label.hpp>

#include "button.h"

#include "../joystick.h"

extern Joystick *joystick;

Setup_Joystick::Setup_Joystick():
    mCalibrateLabel(new gcn::Label("Press the button to start calibration")),
    mCalibrateButton(new Button("Calibrate", "calibrate", this))
{
    setOpaque(false);

    mCalibrateLabel->setPosition(5, 10);
    mCalibrateButton->setPosition(10, 20 + mCalibrateLabel->getHeight());

    add(mCalibrateLabel);
    add(mCalibrateButton);
}

void Setup_Joystick::action(const std::string &event)
{
    if (!joystick) {
        return;
    }

    if (joystick->isCalibrating()) {
        mCalibrateButton->setCaption("Calibrate");
        mCalibrateLabel->setCaption("Press the button to start calibration");
        joystick->finishCalibration();
    } else {
        mCalibrateButton->setCaption("Stop");
        mCalibrateLabel->setCaption("Rotate the stick");
        joystick->startCalibration();
    }
}
