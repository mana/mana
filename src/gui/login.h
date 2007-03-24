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
#include <guichan/keylistener.hpp>

#include "window.h"
#include "../guichanfwd.h"

class LoginData;

/**
 * The login dialog.
 *
 * \ingroup Interface
 */
class LoginDialog : public Window, public gcn::ActionListener,
                    public gcn::KeyListener
{
    public:
        /**
         * Constructor
         *
         * @see Window::Window
         */
        LoginDialog(LoginData *loginData);

        /**
         * Destructor
         */
        ~LoginDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Called when a key is pressed in one of the text fields.
         */
        void keyPressed(gcn::KeyEvent &keyEvent);

    private:
        /**
         * Returns whether submit can be enabled. This is true in the login
         * state, when all necessary fields have some text.
         */
        bool
        canSubmit();

        gcn::TextField *mUserField;
        gcn::TextField *mPassField;
        gcn::CheckBox *mKeepCheck;
        gcn::Button *mOkButton;
        gcn::Button *mCancelButton;
        gcn::Button *mRegisterButton;

        LoginData *mLoginData;
};

#endif
