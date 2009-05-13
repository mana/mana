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

#include "gui/setup_joystick.h"

#include "gui/widgets/button.h"
#include "gui/widgets/checkbox.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layouthelper.h"

#include "configuration.h"
#include "joystick.h"

#include "utils/gettext.h"

extern Joystick *joystick;

Setup_Joystick::Setup_Joystick():
    mCalibrateLabel(new Label(_("Press the button to start calibration"))),
    mCalibrateButton(new Button(_("Calibrate"), "calibrate", this)),
    mJoystickEnabled(new CheckBox(_("Enable joystick")))
{
    setName(_("Joystick"));

    mOriginalJoystickEnabled = !config.getValue("joystickEnabled", false);
    mJoystickEnabled->setSelected(mOriginalJoystickEnabled);

    mJoystickEnabled->addActionListener(this);

    // Do the layout
    LayoutHelper h(this);
    ContainerPlacer place = h.getPlacer(0, 0);

    place(0, 0, mJoystickEnabled);
    place(0, 1, mCalibrateLabel);
    place.getCell().matchColWidth(0, 0);
    place = h.getPlacer(0, 1);
    place(0, 0, mCalibrateButton);

    setDimension(gcn::Rectangle(0, 0, 365, 75));
}

void Setup_Joystick::action(const gcn::ActionEvent &event)
{
    if (!joystick)
    {
        return;
    }

    if (event.getSource() == mJoystickEnabled)
    {
        joystick->setEnabled(mJoystickEnabled->isSelected());
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
        joystick->setEnabled(mOriginalJoystickEnabled);
    }
    mJoystickEnabled->setSelected(mOriginalJoystickEnabled);
}

void Setup_Joystick::apply()
{
    config.setValue("joystickEnabled",
                    joystick ? joystick->isEnabled() : false);
}

