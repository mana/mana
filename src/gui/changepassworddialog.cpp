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

#include "changepassworddialog.h"

#include "client.h"
#include "log.h"

#include "gui/register.h"
#include "gui/okdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/passwordfield.h"
#include "gui/widgets/textfield.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"

#include "net/logindata.h"
#include "net/loginhandler.h"
#include "net/net.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <string>
#include <sstream>

ChangePasswordDialog::ChangePasswordDialog(LoginData *loginData):
    Window(_("Change Password"), true),
    mWrongDataNoticeListener(new WrongDataNoticeListener),
    mLoginData(loginData)
{
    gcn::Label *accountLabel = new Label(
            strprintf(_("Account: %s"), mLoginData->username.c_str()));
    mOldPassField = new PasswordField;
    mFirstPassField = new PasswordField;
    mSecondPassField = new PasswordField;
    mChangePassButton = new Button(_("Change Password"), "change_password",
                                   this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    place(0, 0, accountLabel, 3);
    place(0, 1, new Label(_("Password:")), 3);
    place(0, 2, mOldPassField, 3).setPadding(1);
    place(0, 3, new Label(_("Type new password twice:")), 3);
    place(0, 4, mFirstPassField, 3).setPadding(1);
    place(0, 5, mSecondPassField, 3).setPadding(1);
    place(1, 6, mCancelButton);
    place(2, 6, mChangePassButton);
    reflowLayout(200);

    center();
    setVisible(true);
    mOldPassField->requestFocus();

    mOldPassField->setActionEventId("change_password");
    mFirstPassField->setActionEventId("change_password");
    mSecondPassField->setActionEventId("change_password");
}

ChangePasswordDialog::~ChangePasswordDialog()
{
    delete mWrongDataNoticeListener;
}

void ChangePasswordDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "cancel")
    {
        Client::setState(STATE_CHAR_SELECT);
    }
    else if (event.getId() == "change_password")
    {

        const std::string username = mLoginData->username.c_str();
        const std::string oldPassword = mOldPassField->getText();
        const std::string newFirstPass = mFirstPassField->getText();
        const std::string newSecondPass = mSecondPassField->getText();
        logger->log("ChangePasswordDialog::Password change, Username is %s",
                     username.c_str());

        std::stringstream errorMessage;
        int error = 0;

        unsigned int min = Net::getLoginHandler()->getMinPasswordLength();
        unsigned int max = Net::getLoginHandler()->getMaxPasswordLength();

        // Check old Password
        if (oldPassword.empty())
        {
            // No old password
            errorMessage << _("Enter the old password first.");
            error = 1;
        }
        else if (newFirstPass.length() < min)
        {
            // First password too short
            errorMessage << strprintf(_("The new password needs to be at least"
                                        " %d characters long."), min);
            error = 2;
        }
        else if (newFirstPass.length() > max - 1 )
        {
            // First password too long
            errorMessage << strprintf(_("The new password needs to be less "
                                        "than %d characters long."), max);
            error = 2;
        }
        else if (newFirstPass != newSecondPass)
        {
            // Second Pass mismatch
            errorMessage << _("The new password entries mismatch.");
            error = 3;
        }

        if (error > 0)
        {
            if (error == 1)
            {
                mWrongDataNoticeListener->setTarget(this->mOldPassField);
            }
            else if (error == 2)
            {
                mWrongDataNoticeListener->setTarget(this->mFirstPassField);
            }
            else if (error == 3)
            {
                mWrongDataNoticeListener->setTarget(this->mSecondPassField);
            }

            auto *dlg = new OkDialog(_("Error"), errorMessage.str());
            dlg->addActionListener(mWrongDataNoticeListener);
        }
        else
        {
            // No errors detected, change account password.
            mChangePassButton->setEnabled(false);
            // Set the new password
            mLoginData->password = oldPassword;
            mLoginData->newPassword = newFirstPass;
            Client::setState(STATE_CHANGEPASSWORD_ATTEMPT);
        }
    }
}
