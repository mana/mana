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
#include "radiobutton.h"
#include "textfield.h"
#include "ok_dialog.h"

#include "../net/messagein.h"
#include "../net/messageout.h"
#include "../net/network.h"

//OkDialog *wrongLoginNotice = NULL;
extern SERVER_INFO **server_info;

RegisterDialog::RegisterDialog():
    Window("Register"), mStatus(NET_IDLE)
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
            }
            wrongRegisterNotice = new OkDialog("Error", errorMsg.str(),
                                            wrongDataNoticeListener);
        }
        else
        {
            // No errors detected, register the new user.
            const std::string host(config.getValue("host", "animesites.de"));
            short port = (short)config.getValue("port", 0);
            // Attempt to connect to login server
            openConnection(host.c_str(), port);
            registerButton->setEnabled(false);
            mStatus = NET_CONNECTING;
        }
    }
}

void
RegisterDialog::logic()
{
    switch (mStatus)
    {
        case NET_CONNECTING:
            mStatus = pollConnection();
            break;
        case NET_ERROR:
            logger->log("Register::Unable to connect");
            errorMessage = "Unable to connect to login server";
            state = ERROR_STATE;
            closeConnection();
            logger->log("Connection closed");
            break;
        case NET_DATA:
            if (packetReady())
            {
                checkRegistration();
                closeConnection();
            }
            else
            {
                flush();
            }
            break;
        case NET_CONNECTED:
            logger->log("Connected...");
            std::string user = userField->getText();
            const std::string password = passwordField->getText();
            if (femaleButton->isMarked())
            {
                user += "_F";
            }
            else
            {
                user += "_M";
            }
            attemptRegistration(user, password);
            mStatus = NET_DATA;
            break;
    }
}

void
RegisterDialog::attemptRegistration(const std::string& user,
                                    const std::string& pass)
{
    // Send login infos
    MessageOut outMsg;
    outMsg.writeInt16(0x0064);
    outMsg.writeInt32(0); // client version
    outMsg.writeString(user, 24);
    outMsg.writeString(pass, 24);
    outMsg.writeInt8(0); // unknown
}

void
RegisterDialog::checkRegistration()
{
    // Receive reply
    MessageIn msg = get_next_message();
    if (state == ERROR_STATE)
    {
        return;
    }

    // Login ok
    if (msg.getId() == 0x0069)
    {
        // Skip the length word
        msg.skip(2);

        n_server = (msg.getLength() - 47) / 32;
        server_info = (SERVER_INFO**)malloc(sizeof(SERVER_INFO*) * n_server);

        session_ID1 = msg.readInt32();
        account_ID = msg.readInt32();
        session_ID2 = msg.readInt32();
        msg.skip(30);                           // unknown
        sex = msg.readInt8();

        for (int i = 0; i < n_server; i++)
        {
            server_info[i] = new SERVER_INFO;

            server_info[i]->address = msg.readInt32();
            server_info[i]->port = msg.readInt16();
            server_info[i]->name = msg.readString(20);
            server_info[i]->online_users = msg.readInt32();
            msg.skip(2);                        // unknown

            logger->log("Network: Server: %s (%s:%d)",
                        server_info[i]->name.c_str(),
                        iptostring(server_info[i]->address),
                        server_info[i]->port);
        }
        skip(msg.getLength());

        state = CHAR_SERVER_STATE;
    }
    else if (msg.getId() == 0x006a)
    {
        int loginError = msg.readInt8();
        logger->log("Login::error code: %i", loginError);

        switch (loginError) {
            case 0:
                errorMessage = "Unregistered ID";
                break;
            case 1:
                errorMessage = "Wrong password";
                break;
            case 2:
                errorMessage = "Account expired";
                break;
            case 3:
                errorMessage = "Rejected from server";
                break;
            case 4:
                errorMessage = "You have been blocked by the GM Team";
                break;
            case 9:
                errorMessage = "This account is already logged in";
                break;
        }
        skip(msg.getLength());
        state = ERROR_STATE;
    }
    else {
        skip(msg.getLength());
        logger->log("Login::Unknown error");
        errorMessage = "Unknown error";
        state = ERROR_STATE;
    }
    // Todo: add other packets, also encrypted
}

void
registerInputHandler(SDL_KeyboardEvent *keyEvent)
{
    if (keyEvent->keysym.sym == SDLK_ESCAPE)
    {
        state = EXIT_STATE;
    }
}
