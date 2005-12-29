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

#ifndef _TMW_LOGIN_H
#define _TMW_LOGIN_H

#include <iosfwd>
#include <guichan/actionlistener.hpp>
#include <SDL_events.h>

#include "window.h"
#include "../guichanfwd.h"

/**
 * The login dialog.
 *
 * \ingroup Interface
 */
class LoginDialog : public Window, public gcn::ActionListener {
    public:
        /**
         * Constructor
         *
         * @see Window::Window
         */
        LoginDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string& eventId);

        /**
         * Updates dialog logic.
         */
        void logic();

        // Made them public to have the possibility to request focus
        // from external functions.
        gcn::TextField *userField;
        gcn::TextField *passField;

    private:
        gcn::Label *userLabel;
        gcn::Label *passLabel;
        gcn::Label *serverLabel;
        gcn::TextField *serverField;
        gcn::CheckBox *keepCheck;
        gcn::Button *okButton;
        gcn::Button *cancelButton;
        gcn::Button *registerButton;
        int mStatus;
        bool registration;
        
        void attemptLogin(const std::string& user, const std::string& pass);
        void checkLogin();
};

/**
 * Listener used for handling wrong password.
 */
class WrongPasswordNoticeListener : public gcn::ActionListener {
    public:
        void setLoginDialog(LoginDialog *loginDialog);
        void action(const std::string &eventId);
    private:
        LoginDialog *mLoginDialog;
};

/**
 * Listener used for handling wrong username.
 */
class WrongUsernameNoticeListener : public gcn::ActionListener {
    public:
        void setLoginDialog(LoginDialog *loginDialog);
        void action(const std::string &eventId);
    private:
        LoginDialog *mLoginDialog;
};


/**
 * Handle input
 */
void loginInputHandler(SDL_KeyboardEvent *keyEvent);

#endif
