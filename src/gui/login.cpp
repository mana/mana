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

#include "../net/messagein.h"
#include "../net/messageout.h"
#include "../net/network.h"

SERVER_INFO **server_info;

void
WrongDataNoticeListener::setTarget(gcn::TextField *textField)
{
    this->target = textField;
}

void
WrongDataNoticeListener::action(const std::string &eventId)
{
    if (eventId == "ok")
    {
        // Reset the field
        target->setText("");
        target->setCaretPosition(0);
        target->requestFocus();
    }
}

LoginDialog::LoginDialog():
    Window("Login"), mStatus(NET_IDLE)
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

    wrongDataNoticeListener = NULL;
}

LoginDialog::~LoginDialog()
{
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
            wrongDataNoticeListener = new WrongDataNoticeListener();
            wrongDataNoticeListener->setTarget(this->passField);
            new OkDialog("Error", "Enter your username first",
                     wrongDataNoticeListener);
        }
        else
        {
            const std::string host(config.getValue("host", "animesites.de"));
            short port = (short)config.getValue("port", 0);
            // Attempt to connect to login server
            openConnection(host.c_str(), port);
            okButton->setEnabled(false);
            //cancelButton->setEnabled(false);
            registerButton->setEnabled(false);
            mStatus = NET_CONNECTING;
        }
    }
    else if (eventId == "cancel")
    {
        state = EXIT_STATE;
    }
    else if (eventId == "register")
    {
        state = REGISTER_STATE;
    }
}

void
LoginDialog::logic()
{
    switch (mStatus)
    {
        case NET_CONNECTING:
            mStatus = pollConnection();
            break;
        case NET_ERROR:
            logger->log("Login::Unable to connect");
            errorMessage = "Unable to connect to login server";
            state = ERROR_STATE;
            closeConnection();
            logger->log("Connection closed");
            break;
        case NET_DATA:
            if (packetReady())
            {
                checkLogin();
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
            const std::string password = passField->getText();
            attemptLogin(user, password);
            mStatus = NET_DATA;
            break;
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

void
LoginDialog::attemptLogin(const std::string& user, const std::string& pass)
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
LoginDialog::checkLogin()
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
            case 6:
                errorMessage = "You have been banned for 5 minutes";
                break;
            case 9:
                errorMessage = "This account is already logged in";
                break;
            default:
                errorMessage = "Unknown error";
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
