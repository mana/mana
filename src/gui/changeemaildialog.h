/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#ifndef GUI_CHANGEEMAIL_H
#define GUI_CHANGEEMAIL_H

#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

class LoginData;
class OkDialog;
class WrongDataNoticeListener;

/**
 * The Change email dialog.
 *
 * \ingroup Interface
 */
class ChangeEmailDialog : public Window, public gcn::ActionListener
{
    public:
        ChangeEmailDialog(LoginData *loginData);

        ~ChangeEmailDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * This is used to pass the pointer to where the new email should be
         * put when the dialog finishes.
         */
        static void setEmail(std::string *email);

    private:
        gcn::TextField *mFirstEmailField;
        gcn::TextField *mSecondEmailField;

        gcn::Button *mChangeEmailButton;
        gcn::Button *mCancelButton;

        WrongDataNoticeListener *mWrongDataNoticeListener;

        LoginData *mLoginData;
};

#endif // GUI_CHANGEEMAIL_H
