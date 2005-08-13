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

#include <guichan/sdl/sdlinput.hpp>

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "checkbox.h"
#include "gui.h"
#include "ok_dialog.h"
#include "passwordfield.h"
#include "textfield.h"

#include "../configuration.h"
#include "../graphics.h"
#include "../log.h"
#include "../main.h"
#include "../serverinfo.h"

#include "../net/network.h"

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

void LoginDialog::action(const std::string& eventId)
{
    if (eventId == "ok") {
        const std::string user = userField->getText();
        logger->log("Network: Username is %s", user.c_str());

        // Store config settings
        config.setValue("remember", keepCheck->isMarked());
        if (keepCheck->isMarked()) {
            config.setValue("username", user);
            config.setValue("host", serverField->getText());
        } else {
            config.setValue("username", "");
        }

        // Check login
        if (user.length() == 0) {
            new OkDialog("Error", "Enter your username first");
        } else {
            server_login(user, passField->getText());
            close_session();
        }
    } else if (eventId == "cancel") {
        state = EXIT;
    } else if (eventId == "register") {
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
        if (user.length() == 0) {
            new OkDialog("Error", "Enter a username first");
        } else if (user.length() < 4) {
            new OkDialog("Error", "The username needs to be at least 4 characters");
        } else if (user.length() > LEN_USERNAME -1 ) {
            new OkDialog("Error", "The username needs to be less than 25 characters long.");
        } else {
            server_login(user + "_M", passField->getText());
            close_session();
        }
    }
}


void login()
{
    LoginDialog *dialog = new LoginDialog();

    while (state == LOGIN)
    {
        // Handle SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state = EXIT;
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        state = EXIT;
                    }
                    break;
            }

            guiInput->pushInput(event);
        }

        gui->logic();

        guiGraphics->drawImage(login_wallpaper, 0, 0);
        gui->draw();
        guiGraphics->updateScreen();
    }

    delete dialog;
}


void server_login(const std::string& user, const std::string& pass) {
    strncpy(username, user.c_str(), LEN_USERNAME);
    strncpy(password, pass.c_str(), LEN_PASSWORD);
    int ret;

    // Connect to login server
    ret = open_session(
            config.getValue("host", "animesites.de").c_str(),
            (short)config.getValue("port", 0));

    if (ret == SOCKET_ERROR) {
        state = LOGIN;
        new OkDialog("Error", "Unable to connect to login server");
        return;
    }

    // Send login infos

    WFIFOW(0) = net_w_value(0x0064);

    WFIFOL(2) = 0;
    memcpy(WFIFOP(6), username, 24);
    memcpy(WFIFOP(30), password, 24);
    WFIFOB(54) = 0;
    WFIFOSET(55);

    while ((in_size < 23) || (out_size > 0)) {
        flush();
    }

    if (RFIFOW(0) == 0x0069) {
        while (in_size < RFIFOW(2)) {
            flush();
        }
        n_server = (RFIFOW(2) - 47) / 32;
        server_info = (SERVER_INFO*)malloc(sizeof(SERVER_INFO) * n_server);
        account_ID = RFIFOL(8);
        session_ID1 = RFIFOL(4);
        session_ID2 = RFIFOL(12);
        sex = RFIFOB(46);
        for (int i = 0; i < n_server; i++) {
            server_info[i].address = RFIFOL(47 + 32 * i);
            memcpy(server_info[i].name, RFIFOP(47 + 32 * i + 6), 20);
            server_info[i].online_users = RFIFOW(47 + 32 * i + 26);
            server_info[i].port = RFIFOW(47 + 32 * i + 4);
            state = CHAR_SERVER;
        }
        logger->log("Network: Server: %s (%s:%d)", server_info[0].name,
                iptostring(server_info[0].address),
                server_info[0].port);
        RFIFOSKIP(RFIFOW(2));
    }
    else if (RFIFOW(0) == 0x006a) {
        logger->log("Login::error code: %i", RFIFOB(2));
        switch (RFIFOB(2)) {
            case 0:
                new OkDialog("Error", "Unregistered ID");
                break;
            case 1:
                new OkDialog("Error", "Wrong password");
                break;
            case 2:
                new OkDialog("Error", "This ID is expired");
                break;
            case 3:
                new OkDialog("Error", "Rejected from server");
                break;
            case 4:
                new OkDialog("Error", "You have been blocked by the GM Team");
                break;
            case 9:
                new OkDialog("Error", "The username does already exist.");
                break;
        }
        state = LOGIN;
        RFIFOSKIP(23);
    }
    else {
        new OkDialog("Error", "Unknown error");
    }
    // Todo: add other packets, also encrypted
}
