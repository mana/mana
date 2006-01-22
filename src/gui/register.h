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

#ifndef _TMW_REGISTER_H
#define _TMW_REGISTER_H

#include <iosfwd>
#include <guichan/actionlistener.hpp>

#include "window.h"
#include "../guichanfwd.h"

class LoginData;
class OkDialog;
class WrongDataNoticeListener;

/**
 * The login dialog.
 *
 * \ingroup Interface
 */
class RegisterDialog : public Window, public gcn::ActionListener {
    public:
        /**
         * Constructor
         *
         * @see Window::Window
         */
        RegisterDialog(LoginData *loginData);

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string& eventId);

        // Made them public to have the possibility to request focus
        // from external functions.
        gcn::TextField *userField;
        gcn::TextField *passwordField;
        gcn::TextField *confirmField;

    private:
        gcn::Label *userLabel;
        gcn::Label *passwordLabel;
        gcn::Label *confirmLabel;
        gcn::Label *serverLabel;
        gcn::TextField *serverField;
        gcn::Button *registerButton;
        gcn::Button *cancelButton;
        gcn::RadioButton *maleButton;
        gcn::RadioButton *femaleButton;

        WrongDataNoticeListener *wrongDataNoticeListener;
        OkDialog *wrongRegisterNotice;

        LoginData *mLoginData;
};

#endif
