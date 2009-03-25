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

#include "changepassworddialog.h"

#include <string>
#include <sstream>

#include <guichan/widgets/label.hpp>

#include "../main.h"
#include "../log.h"
#include "../logindata.h"

#include "button.h"
#include "register.h"
#include "passwordfield.h"
#include "textfield.h"
#include "ok_dialog.h"

#include "../utils/gettext.h"
#include "../utils/strprintf.h"

ChangePasswordDialog::ChangePasswordDialog(Window *parent, LoginData *loginData):
    Window(_("Change Password"), true, parent),
    mWrongDataNoticeListener(new WrongDataNoticeListener()),
    mLoginData(loginData)
{
    gcn::Label *accountLabel = new gcn::Label(strprintf(_("Account: %s"),
                                              mLoginData->username.c_str()));
    gcn::Label *oldPassLabel = new gcn::Label(_("Password:"));
    mOldPassField = new PasswordField();
    gcn::Label *newPassLabel = new gcn::Label(_("Type New Password twice:"));
    mFirstPassField = new PasswordField();
    mSecondPassField = new PasswordField();
    mChangePassButton = new Button(_("Change Password"), "change_password", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    const int width = 200;
    const int height = 170;
    setContentSize(width, height);

    accountLabel->setPosition(5, 5);
    accountLabel->setWidth(130);
    oldPassLabel->setPosition(
            5, accountLabel->getY() + accountLabel->getHeight() + 7);
    oldPassLabel->setWidth(130);

    mOldPassField->setPosition(
            5, oldPassLabel->getY() + oldPassLabel->getHeight() + 7);
    mOldPassField->setWidth(130);

    newPassLabel->setPosition(
            5, mOldPassField->getY() + mOldPassField->getHeight() + 7);
    newPassLabel->setWidth(width - 5);

    mFirstPassField->setPosition(
            5, newPassLabel->getY() + newPassLabel->getHeight() + 7);
    mFirstPassField->setWidth(130);

    mSecondPassField->setPosition(
            5, mFirstPassField->getY() + mFirstPassField->getHeight() + 7);
    mSecondPassField->setWidth(130);

    mCancelButton->setPosition(
            width - 5 - mCancelButton->getWidth(),
            height - 5 - mCancelButton->getHeight());
    mChangePassButton->setPosition(
            mCancelButton->getX() - 5 - mChangePassButton->getWidth(),
            mCancelButton->getY());

    add(accountLabel);
    add(oldPassLabel);
    add(mOldPassField);
    add(newPassLabel);
    add(mFirstPassField);
    add(mSecondPassField);
    add(mChangePassButton);
    add(mCancelButton);

    setLocationRelativeTo(getParent());
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

void
ChangePasswordDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "cancel")
    {
        scheduleDelete();
    }
    else if (event.getId() == "change_password")
    {

        const std::string username = mLoginData->username.c_str();
        const std::string oldPassword = mOldPassField->getText();
        const std::string newFirstPass = mFirstPassField->getText();
        const std::string newSecondPass = mSecondPassField->getText();
        logger->log("ChangePasswordDialog::Password change, Username is %s",
                     username.c_str());

        std::stringstream errorMsg;
        int error = 0;

        // Check old Password
        if (oldPassword.empty())
        {
            // No old password
            errorMsg << "Enter the old Password first.";
            error = 1;
        }
        else if (newFirstPass.length() < LEN_MIN_PASSWORD)
        {
            // First password too short
            errorMsg << "The new password needs to be at least "
                     << LEN_MIN_PASSWORD
                     << " characters long.";
            error = 2;
        }
        else if (newFirstPass.length() > LEN_MAX_PASSWORD - 1 )
        {
            // First password too long
            errorMsg << "The new password needs to be less than "
                     << LEN_MAX_PASSWORD
                     << " characters long.";
            error = 2;
        }
        else if (newFirstPass != newSecondPass)
        {
            // Second Pass mismatch
            errorMsg << "The new password entries mismatch.";
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

            OkDialog *dlg = new OkDialog("Error", errorMsg.str());
            dlg->addActionListener(mWrongDataNoticeListener);
        }
        else
        {
            // No errors detected, change account password.
            mChangePassButton->setEnabled(false);
            // Set the new password
            mLoginData->password = oldPassword;
            mLoginData->newPassword = newFirstPass;
            state = STATE_CHANGEPASSWORD_ATTEMPT;
            scheduleDelete();
        }

    }
}
