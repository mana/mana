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

#include "register.h"

#include <string>
#include <sstream>

#include <guichan/widgets/label.hpp>

#include "../main.h"
#include "../configuration.h"
#include "../log.h"
#include "../logindata.h"

#include "button.h"
#include "checkbox.h"
#include "login.h"
#include "passwordfield.h"
#include "radiobutton.h"
#include "textfield.h"
#include "ok_dialog.h"

RegisterDialog::RegisterDialog(LoginData *loginData):
    Window("Register"), mLoginData(loginData)
{
    userLabel = new gcn::Label("Name:");
    passwordLabel = new gcn::Label("Password:");
    confirmLabel = new gcn::Label("Confirm:");
    serverLabel = new gcn::Label("Server:");
    userField = new TextField("player");
    passwordField = new PasswordField();
    confirmField = new PasswordField();
    serverField = new TextField();
    maleButton = new RadioButton("Male", "sex", true);
    maleButton->setEnabled(false);
    femaleButton = new RadioButton("Female", "sex", false);
    femaleButton->setEnabled(false);
    registerButton = new Button("Register");
    cancelButton = new Button("Cancel");
    
    int width = 200;
    int height = 150;
    setContentSize(width, height);
    
    userField->setPosition(65, 5);
    userField->setWidth(130);
    passwordField->setPosition(
            65, userField->getY() + userField->getHeight() + 7);
    passwordField->setWidth(130);
    confirmField->setPosition(
            65, passwordField->getY() + passwordField->getHeight() + 7);
    confirmField->setWidth(130);
    serverField->setPosition(
            65, 23 + confirmField->getY() + confirmField->getHeight() + 7);
    serverField->setWidth(130);

    userLabel->setPosition(5, userField->getY() + 1);
    passwordLabel->setPosition(5, passwordField->getY() + 1);
    confirmLabel->setPosition(5, confirmField->getY() + 1);
    serverLabel->setPosition(5, serverField->getY() + 1);
    
    femaleButton->setPosition(width - femaleButton->getWidth() - 10,
                              confirmField->getY() + confirmField->getHeight() + 7);
    maleButton->setPosition(femaleButton->getX() - maleButton->getWidth() - 5,
                            femaleButton->getY());
                            
    registerButton->setPosition(5, height - registerButton->getHeight() - 5);
    cancelButton->setPosition(10 + registerButton->getWidth(),
                              registerButton->getY());


    /*userField->setEventId("register");
    passwordField->setEventId("register");
    confirmField->setEventId("register");
    serverField->setEventId("register");*/
    registerButton->setEventId("register");
    cancelButton->setEventId("cancel");


    /*userField->addActionListener(this);
    passwordField->addActionListener(this);
    confirmField->addActionListener(this);
    serverField->addActionListener(this);*/
    registerButton->addActionListener(this);
    cancelButton->addActionListener(this);

    add(userLabel);
    add(passwordLabel);
    add(serverLabel);
    add(confirmLabel);
    add(userField);
    add(passwordField);
    add(confirmField);
    add(serverField);
    add(maleButton);
    add(femaleButton);
    add(registerButton);
    add(cancelButton);

    setLocationRelativeTo(getParent());
    userField->requestFocus();
    userField->setCaretPosition(userField->getText().length());

    serverField->setText(config.getValue("host", ""));
    
    wrongDataNoticeListener = NULL;
    wrongRegisterNotice = NULL;
}

void
RegisterDialog::action(const std::string& eventId)
{
    if (eventId == "cancel")
    {
        state = EXIT_STATE;
    }
    else if (eventId == "register")
    {
        const std::string user = userField->getText();
        logger->log("RegisterDialog::register Username is %s", user.c_str());

        std::stringstream errorMsg;
        int error = 0;

        // Check login
        if (user.length() == 0)
        {
            // No username
            errorMsg << "Enter your username first.";
            error = 1;
        }
        else if (user.length() < LEN_MIN_USERNAME)
        {
            // Name too short
            errorMsg << "The username needs to be at least "
                     << LEN_MIN_USERNAME
                     << " characters long.";
            error = 1;
        }
        else if (user.length() > LEN_MAX_USERNAME - 1 )
        {
            // Name too long
            errorMsg << "The username needs to be less than "
                     << LEN_MAX_USERNAME
                     << " characters long.";
            error = 1;
        }
        else if (passwordField->getText().length() < LEN_MIN_PASSWORD)
        {
            // Pass too short
            errorMsg << "The password needs to be at least "
                     << LEN_MIN_PASSWORD
                     << " characters long.";
            error = 2;
        }
        else if (passwordField->getText().length() > LEN_MAX_PASSWORD - 1 )
        {
            // Pass too long
            errorMsg << "The password needs to be less than "
                     << LEN_MAX_PASSWORD
                     << " characters long.";
            error = 2;
        }
        else if (passwordField->getText() != confirmField->getText())
        {
            // Password does not match with the confirmation one
            errorMsg << "Passwords do not match.";
            error = 2;
        }
        
        if (error > 0)
        {
            wrongDataNoticeListener = new WrongDataNoticeListener();
            if (error == 1)
            {
                wrongDataNoticeListener->setTarget(this->userField);
            }
            else if (error == 2)
            {
                wrongDataNoticeListener->setTarget(this->passwordField);
                // Reset password confirmation
                confirmField->setText("");
            }

            if (wrongRegisterNotice)
            {
                delete wrongRegisterNotice;
                wrongRegisterNotice = NULL;
            }
            wrongRegisterNotice = new OkDialog("Error", errorMsg.str());
            wrongRegisterNotice->addActionListener(wrongDataNoticeListener);
        }
        else
        {
            // No errors detected, register the new user.
            registerButton->setEnabled(false);

            mLoginData->hostname = config.getValue("host", "animesites.de");
            mLoginData->port = (short)config.getValue("port", 0);
            mLoginData->username = userField->getText();
            mLoginData->password = passwordField->getText();
            mLoginData->username += femaleButton->isMarked() ? "_F" : "_M";

            state = ACCOUNT_STATE;
        }
    }
}
