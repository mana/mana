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

#ifndef REGISTER_H
#define REGISTER_H

#include <string>

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

#include "window.h"

class LoginData;
class OkDialog;
class WrongDataNoticeListener;

/**
 * The registration dialog.
 *
 * \ingroup Interface
 */
class RegisterDialog : public Window, public gcn::ActionListener,
                       public gcn::KeyListener
{
    public:
        /**
         * Constructor. Name, password and server fields will be initialized to
         * the information already present in the LoginData instance.
         *
         * @see Window::Window
         */
        RegisterDialog(LoginData *loginData);

        /**
         * Destructor
         */
        ~RegisterDialog();

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
         * Returns whether submit can be enabled. This is true in the register
         * state, when all necessary fields have some text.
         */
        bool canSubmit() const;

        /**
         * Function to decide whether string is an unsigned short or not
         *
         * @param str the string to parse
         *
         * @return true if str is an unsigned short, false otherwise
         */
        static bool isUShort(const std::string &str);

        /**
         * Converts string to an unsigned short (undefined if invalid)
         *
         * @param str the string to parse
         *
         * @return the value str represents
         */
        static unsigned short getUShort(const std::string &str);

        gcn::TextField *mUserField;
        gcn::TextField *mPasswordField;
        gcn::TextField *mConfirmField;
        gcn::TextField *mServerField;
        gcn::TextField *mPortField;

        gcn::Button *mRegisterButton;
        gcn::Button *mCancelButton;
        gcn::RadioButton *mMaleButton;
        gcn::RadioButton *mFemaleButton;

        WrongDataNoticeListener *mWrongDataNoticeListener;

        LoginData *mLoginData;
};

#endif
