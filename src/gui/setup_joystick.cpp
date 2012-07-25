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

#include "gui/setup_joystick.h"

#include "configuration.h"
#include "joystick.h"

#include "gui/widgets/button.h"
#include "gui/widgets/checkbox.h"
#include "gui/widgets/label.h"

#include "utils/gettext.h"

extern Joystick *joystick;

Setup_Joystick::Setup_Joystick():
    mCalibrateLabel(new Label(_("Press the button to start calibration"))),
    mCalibrateButton(new Button(_("Calibrate"), "calibrate", this)),
    mJoystickCheckBox(new CheckBox(_("Enable joystick")))
{
    setName(_("Joystick"));

    mJoystickEnabled = config.getBoolValue("joystickEnabled");
    mJoystickCheckBox->setSelected(mJoystickEnabled);

    mJoystickCheckBox->addActionListener(this);

    // Do the layout
    place(0, 0, mJoystickCheckBox, 2);
    place(0, 1, mCalibrateLabel, 2);
    place(0, 2, mCalibrateButton);
}

void Setup_Joystick::action(const gcn::ActionEvent &event)
{
    if (!joystick)
    {
        return;
    }

    if (event.getSource() == mJoystickCheckBox)
    {
        joystick->setEnabled(mJoystickCheckBox->isSelected());
    }
    else
    {
        if (joystick->isCalibrating())
        {
            mCalibrateButton->setCaption(_("Calibrate"));
            mCalibrateLabel->setCaption
                (_("Press the button to start calibration"));
            joystick->finishCalibration();
        }
        else
        {
            mCalibrateButton->setCaption(_("Stop"));
            mCalibrateLabel->setCaption(_("Rotate the stick"));
            joystick->startCalibration();
        }
    }
}

void Setup_Joystick::cancel()
{
    if (joystick)
    {
        joystick->setEnabled(mJoystickEnabled);
    }
    mJoystickCheckBox->setSelected(mJoystickEnabled);
}

void Setup_Joystick::apply()
{
    config.setValue("joystickEnabled",
                    joystick ? joystick->isEnabled() : false);
}

