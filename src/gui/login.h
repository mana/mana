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

#ifndef LOGIN_H
#define LOGIN_H

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>
#include <guichan/listmodel.hpp>

#include <string>
#include <vector>

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
        bool canSubmit();

#ifdef EATHENA_SUPPORT
        /**
         * Function to decide whether string is an unsigned short or not
         *
         * @param str the string to parse
         *
         * @return true is str is an unsigned short, false otherwise
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

#endif
        gcn::TextField *mUserField;
        gcn::TextField *mPassField;
#ifdef EATHENA_SUPPORT
        gcn::TextField *mServerField;
        gcn::TextField *mPortField;
        gcn::DropDown *mServerDropDown;
#endif
        gcn::CheckBox *mKeepCheck;
        gcn::Button *mOkButton;
        gcn::Button *mCancelButton;
        gcn::Button *mRegisterButton;

        LoginData *mLoginData;

        /**
         * Helper class to keep a list of all the recent entries for the
         * dropdown
         */
        class DropDownList : public gcn::ListModel
        {
            private:
                std::vector<std::string> mServers;
                std::vector<std::string> mPorts;
                std::string mConfigPrefix;
                int mMaxEntries;
                void saveEntry(const std::string &server,
                               const std::string &port, int &saved);
            public:
                DropDownList(std::string prefix,
                             std::vector<std::string> dfltServer,
                             std::vector<std::string> dfltPort,
                             int maxEntries);
                void save(const std::string &server, const std::string &port);
                int getNumberOfElements();
                std::string getElementAt(int i);
                std::string getServerAt(int i);
                std::string getPortAt(int i);
        };
        DropDownList *mServerList;
};

#endif
