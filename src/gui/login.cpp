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

#include "login.h"

#include <string>
#include <sstream>
#include <SDL.h>

#include <guichan/widgets/label.hpp>

#include "../main.h"
#include "../configuration.h"
#include "../graphics.h"
#include "../log.h"
#include "../serverinfo.h"

#include "button.h"
#include "checkbox.h"
#include "passwordfield.h"
#include "textfield.h"
#include "ok_dialog.h"

#include "../net/messagein.h"
#include "../net/messageout.h"
#include "../net/network.h"

OkDialog *wrongLoginNotice = NULL;
SERVER_INFO **server_info;

WrongUsernameNoticeListener wrongUsernameNoticeListener;
WrongPasswordNoticeListener wrongPasswordNoticeListener;

void
WrongPasswordNoticeListener::setLoginDialog(LoginDialog *loginDialog)
{
    mLoginDialog = loginDialog;
}

void
WrongPasswordNoticeListener::action(const std::string &eventId)
{
    if (eventId == "ok")
    {
        // Reset the password and put the caret ready to retype it.
        mLoginDialog->passField->setText("");
        mLoginDialog->passField->setCaretPosition(0);
        mLoginDialog->passField->requestFocus();
        wrongLoginNotice = NULL;
    }
}

void
WrongUsernameNoticeListener::setLoginDialog(LoginDialog *loginDialog)
{
    mLoginDialog = loginDialog;
}

void
WrongUsernameNoticeListener::action(const std::string &eventId)
{
    if (eventId == "ok")
    {
        // Set the focus on the username Field
        mLoginDialog->userField->setCaretPosition(LEN_MAX_USERNAME - 1);
        mLoginDialog->userField->requestFocus();
        wrongLoginNotice = NULL;
    }
}

LoginDialog::LoginDialog():
    Window("Login")
{
    userLabel = new gcn::Label("Name:");
    passLabel = new gcn::Label("Password:");
    serverLabel = new gcn::Label("Server:");
    userField = new TextField("player");
    passField = new PasswordField();
    serverField = new TextField();
    keepCheck = new CheckBox("Keep", false);
    okButton = new Button("OK");
    cancelButton = new Button("Cancel");
    registerButton = new Button("Register");

    setContentSize(200, 100);

    userLabel->setPosition(5, 5);
    passLabel->setPosition(5, 14 + userLabel->getHeight());
    serverLabel->setPosition(
            5, 23 + userLabel->getHeight() + passLabel->getHeight());
    userField->setPosition(65, 5);
    passField->setPosition(65, 14 + userLabel->getHeight());
    serverField->setPosition(
            65, 23 + userLabel->getHeight() + passLabel->getHeight());
    userField->setWidth(130);
    passField->setWidth(130);
    serverField->setWidth(130);
    keepCheck->setPosition(4, 77);
    keepCheck->setMarked(config.getValue("remember", 0));
    cancelButton->setPosition(
            200 - cancelButton->getWidth() - 5,
            100 - cancelButton->getHeight() - 5);
    okButton->setPosition(
            cancelButton->getX() - okButton->getWidth() - 5,
            100 - okButton->getHeight() - 5);
    registerButton->setPosition(keepCheck->getX() + keepCheck->getWidth() + 10,
            100 - registerButton->getHeight() - 5);

    userField->setEventId("ok");
    passField->setEventId("ok");
    serverField->setEventId("ok");
    okButton->setEventId("ok");
    cancelButton->setEventId("cancel");
    registerButton->setEventId("register");

    userField->addActionListener(this);
    passField->addActionListener(this);
    serverField->addActionListener(this);
    keepCheck->addActionListener(this);
    okButton->addActionListener(this);
    cancelButton->addActionListener(this);
    registerButton->addActionListener(this);

    add(userLabel);
    add(passLabel);
    add(serverLabel);
    add(userField);
    add(passField);
    add(serverField);
    add(keepCheck);
    add(okButton);
    add(cancelButton);
    add(registerButton);

    setLocationRelativeTo(getParent());
    userField->requestFocus();
    userField->setCaretPosition(userField->getText().length());

    if (config.getValue("remember", 0) != 0) {
        if (config.getValue("username", "") != "") {
            userField->setText(config.getValue("username", ""));
            passField->requestFocus();
        }
    }

    serverField->setText(config.getValue("host", ""));

    wrongUsernameNoticeListener.setLoginDialog(this);
    wrongPasswordNoticeListener.setLoginDialog(this);
}

LoginDialog::~LoginDialog()
{
    delete userLabel;
    delete passLabel;
    delete serverLabel;
    delete userField;
    delete passField;
    delete serverField;
    delete keepCheck;
    delete okButton;
    delete cancelButton;
    delete registerButton;
}

void
LoginDialog::action(const std::string& eventId)
{
    if (eventId == "ok")
    {
        const std::string user = userField->getText();
        logger->log("Network: Username is %s", user.c_str());

        // Store config settings
        config.setValue("remember", keepCheck->isMarked());

        if (keepCheck->isMarked())
        {
            config.setValue("username", user);
            config.setValue("host", serverField->getText());
        }
        else
        {
            config.setValue("username", "");
        }

        // Check login
        if (user.length() == 0)
        {
            wrongLoginNotice = new OkDialog("Error",
                                            "Enter your username first",
                                            &wrongUsernameNoticeListener);
        }
        else
        {
            int ret = attemptLogin(user, passField->getText());

            if (ret == LOGIN_WRONG_PASSWORD)
            {
                wrongLoginNotice = new OkDialog("Error", "Wrong Password",
                                                &wrongPasswordNoticeListener);
            }
            else if (ret != LOGIN_OK)
            {
                std::string errorMsg = "Unknown error.";

                switch (ret)
                {
                    case LOGIN_UNREGISTERED_ID:
                        errorMsg = "Unregistered ID.";
                        break;
                    case LOGIN_EXPIRED:
                        errorMsg = "This ID is expired";
                        break;
                    case LOGIN_REJECTED:
                        errorMsg = "Rejected from server";
                        break;
                    case LOGIN_BLOCKED:
                        errorMsg = "You have been blocked by the GM Team";
                        break;
                    case LOGIN_USERNAME_TWICE:
                        errorMsg = "The username does already exist.";
                        break;
                }

                wrongLoginNotice = new OkDialog("Error", errorMsg);
            }
        }
    }
    else if (eventId == "cancel")
    {
        state = EXIT_STATE;
    }
    else if (eventId == "register")
    {
        const std::string user = userField->getText();
        logger->log("LoginDialog::register Username is %s", user.c_str());

        // Store config settings
        config.setValue("remember", keepCheck->isMarked());
        if (keepCheck->isMarked()) {
            config.setValue("username", user);
        } else {
            config.setValue("username", "");
        }

        // Check login
        if (user.length() == 0)
        {
            // No username
            wrongLoginNotice = new OkDialog("Error",
                                            "Enter your username first.",
                                            &wrongUsernameNoticeListener);
        }
        else if (user.length() < LEN_MIN_USERNAME)
        {
            // Name too short
            std::stringstream errorMsg;
            errorMsg << "The username needs to be at least "
                     << LEN_MIN_USERNAME
                     << " characters long.";
            wrongLoginNotice = new OkDialog("Error", errorMsg.str(),
                                            &wrongUsernameNoticeListener);
        }
        else if (user.length() > LEN_MAX_USERNAME - 1 )
        {
            // Name too long
            std::stringstream errorMsg;
            errorMsg << "The username needs to be less than "
                     << LEN_MAX_USERNAME
                     << " characters long.";
            wrongLoginNotice = new OkDialog("Error", errorMsg.str(),
                                            &wrongUsernameNoticeListener);
        }
        else if (passField->getText().length() < LEN_MIN_PASSWORD)
        {
            // Pass too short
            std::stringstream errorMsg;
            errorMsg << "The password needs to be at least "
                     << LEN_MIN_PASSWORD
                     << " characters long.";
            wrongLoginNotice = new OkDialog("Error", errorMsg.str(),
                                            &wrongPasswordNoticeListener);
        }
        else if (passField->getText().length() > LEN_MAX_PASSWORD - 1 )
        {
            // Pass too long
            std::stringstream errorMsg;
            errorMsg << "The password needs to be less than "
                     << LEN_MAX_PASSWORD
                     << " characters long.";
            wrongLoginNotice = new OkDialog("Error", errorMsg.str(),
                                            &wrongPasswordNoticeListener);
        }
        else
        {
            // No errors detected, register the new user.
            attemptLogin(user + "_M", passField->getText());
        }
    }
}

void
loginInputHandler(SDL_KeyboardEvent *keyEvent)
{
    if (keyEvent->keysym.sym == SDLK_ESCAPE)
    {
        state = EXIT_STATE;
    }
}

int
attemptLogin(const std::string& user, const std::string& pass)
{
    int ret;

    // Connect to login server
    ret = open_session(
            config.getValue("host", "animesites.de").c_str(),
            (short)config.getValue("port", 0));

    if (ret == -1) {
        state = LOGIN_STATE;
        wrongLoginNotice = new OkDialog("Error",
                                        "Unable to connect to login server");
        return LOGIN_NO_CONNECTION;
    }


    // Send login infos
    MessageOut outMsg;
    outMsg.writeShort(0x0064);
    outMsg.writeLong(0); // client version
    outMsg.writeString(user, 24);
    outMsg.writeString(pass, 24);
    outMsg.writeByte(0); // unknown

    // Receive reply
    MessageIn msg = get_next_message();
    if (state == ERROR_STATE)
    {
        close_session();
        return LOGIN_UNKNOWN_ERROR;
    }

    // Login ok
    if (msg.getId() == 0x0069)
    {
        // Skip the length word
        msg.skip(2);

        n_server = (msg.getLength() - 47) / 32;
        server_info = (SERVER_INFO**)malloc(sizeof(SERVER_INFO*) * n_server);

        session_ID1 = msg.readLong();
        account_ID = msg.readLong();
        session_ID2 = msg.readLong();
        msg.skip(30);                           // unknown
        sex = msg.readByte();

        for (int i = 0; i < n_server; i++)
        {
            server_info[i] = new SERVER_INFO;

            server_info[i]->address = msg.readLong();
            server_info[i]->port = msg.readShort();
            server_info[i]->name = msg.readString(20);
            server_info[i]->online_users = msg.readLong();
            msg.skip(2);                        // unknown

            logger->log("Network: Server: %s (%s:%d)",
                        server_info[i]->name.c_str(),
                        iptostring(server_info[i]->address),
                        server_info[i]->port);
        }

        state = CHAR_SERVER_STATE;

        skip(msg.getLength());
        ret = LOGIN_OK;
    }
    else if (msg.getId() == 0x006a)
    {
        int loginError = msg.readByte();
        logger->log("Login::error code: %i", loginError);
        ret = 0;
        switch (loginError) {
            case 0:
                ret = LOGIN_UNREGISTERED_ID;
                break;
            case 1:
                ret = LOGIN_WRONG_PASSWORD;
                break;
            case 2:
                ret = LOGIN_EXPIRED;
                break;
            case 3:
                ret = LOGIN_REJECTED;
                break;
            case 4:
                ret = LOGIN_BLOCKED;
                break;
            case 9:
                ret = LOGIN_USERNAME_TWICE;
                break;
        }
        skip(msg.getLength());
        state = LOGIN_STATE;
    }
    else {
        skip(msg.getLength());
        state = LOGIN_STATE;
        ret = LOGIN_UNKNOWN_ERROR;
    }
    // Todo: add other packets, also encrypted

    close_session();
    return ret;
}
