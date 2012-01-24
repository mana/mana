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

#ifndef GUI_SETUP_JOYSTICK_H
#define GUI_SETUP_JOYSTICK_H

#include "guichanfwd.h"

#include "gui/widgets/setuptab.h"

#include <guichan/actionlistener.hpp>

class Setup_Joystick : public SetupTab, public gcn::ActionListener
{
    public:
        Setup_Joystick();

        void apply();
        void cancel();

        void action(const gcn::ActionEvent &event);

    private:
        gcn::Label *mCalibrateLabel;
        gcn::Button *mCalibrateButton;
        bool mJoystickEnabled;
        gcn::CheckBox *mJoystickCheckBox;
};

#endif
