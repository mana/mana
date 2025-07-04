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

#include "gui/changeemaildialog.h"

#include "client.h"
#include "log.h"

#include "gui/register.h"
#include "gui/okdialog.h"

#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/textfield.h"

#include "net/logindata.h"
#include "net/loginhandler.h"
#include "net/net.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <string>
#include <sstream>

ChangeEmailDialog::ChangeEmailDialog(LoginData *loginData):
    Window(_("Change Email Address"), true),
    mWrongDataNoticeListener(new WrongDataNoticeListener),
    mLoginData(loginData)
{
    gcn::Label *accountLabel = new Label(strprintf(_("Account: %s"),
                                                   mLoginData->username.c_str()));
    gcn::Label *newEmailLabel = new Label(_("Type new email address twice:"));
    mFirstEmailField = new TextField;
    mSecondEmailField = new TextField;
    mChangeEmailButton = new Button(_("Change Email Address"), "change_email", this);
    mCancelButton = new Button(_("Cancel"), "cancel", this);

    const int width = 200;
    const int height = 130;
    setContentSize(width, height);

    accountLabel->setPosition(5, 5);
    accountLabel->setWidth(130);

    newEmailLabel->setPosition(
            5, accountLabel->getY() + accountLabel->getHeight() + 7);
    newEmailLabel->setWidth(width - 5);

    mFirstEmailField->setPosition(
            5, newEmailLabel->getY() + newEmailLabel->getHeight() + 7);
    mFirstEmailField->setWidth(130);

    mSecondEmailField->setPosition(
            5, mFirstEmailField->getY() + mFirstEmailField->getHeight() + 7);
    mSecondEmailField->setWidth(130);

    mCancelButton->setPosition(
            width - 5 - mCancelButton->getWidth(),
            height - 5 - mCancelButton->getHeight());
    mChangeEmailButton->setPosition(
            mCancelButton->getX() - 5 - mChangeEmailButton->getWidth(),
            mCancelButton->getY());

    add(accountLabel);
    add(newEmailLabel);
    add(mFirstEmailField);
    add(mSecondEmailField);
    add(mChangeEmailButton);
    add(mCancelButton);

    center();
    setVisible(true);
    mFirstEmailField->requestFocus();

    mFirstEmailField->setActionEventId("change_email");
    mSecondEmailField->setActionEventId("change_email");
}

ChangeEmailDialog::~ChangeEmailDialog()
{
    delete mWrongDataNoticeListener;
}

void ChangeEmailDialog::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "cancel")
    {
        Client::setState(STATE_CHAR_SELECT);
    }
    else if (event.getId() == "change_email")
    {

        const std::string username = mLoginData->username.c_str();
        const std::string newFirstEmail = mFirstEmailField->getText();
        const std::string newSecondEmail = mSecondEmailField->getText();
        Log::info("ChangeEmailDialog::Email change, Username is %s",
                  username.c_str());

        std::stringstream errorMessage;
        int error = 0;

        unsigned int min = Net::getLoginHandler()->getMinPasswordLength();
        unsigned int max = Net::getLoginHandler()->getMaxPasswordLength();

        if (newFirstEmail.length() < min)
        {
            // First email address too short
            errorMessage << strprintf(_("The new email address needs to be at "
                                        "least %d characters long."), min);
            error = 1;
        }
        else if (newFirstEmail.length() > max - 1 )
        {
            // First email address too long
            errorMessage << strprintf(_("The new email address needs to be "
                                        "less than %d characters long."), max);
            error = 1;
        }
        else if (newFirstEmail != newSecondEmail)
        {
            // Second Pass mismatch
            errorMessage << _("The email address entries mismatch.");
            error = 2;
        }

        if (error > 0)
        {
            if (error == 1)
            {
                mWrongDataNoticeListener->setTarget(this->mFirstEmailField);
            }
            else if (error == 2)
            {
                mWrongDataNoticeListener->setTarget(this->mSecondEmailField);
            }

            auto *dlg = new OkDialog(_("Error"), errorMessage.str());
            dlg->addActionListener(mWrongDataNoticeListener);
        }
        else
        {
            // No errors detected, change account password.
            mChangeEmailButton->setEnabled(false);
            // Set the new email address
            mLoginData->email = newFirstEmail;
            Client::setState(STATE_CHANGEEMAIL_ATTEMPT);
        }
    }
}
