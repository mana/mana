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
 *  By ElvenProgrammer aka Eugenio Favalli (umperio@users.sourceforge.net)
 */

#include "login.h"
#include "gui.h"
#include "button.h"
#include "checkbox.h"
#include "window.h"
#include "../graphic/graphic.h"

// Dialog parts
gcn::Container *dialog;
gcn::Label *userLabel;
gcn::Label *passLabel;
gcn::TextField *userField;
gcn::TextField *passField;
gcn::CheckBox *keepCheck;
gcn::Button *okButton;
gcn::Button *cancelButton;

void LoginActionListener::action(const std::string& eventId)
{
    if (eventId == "ok") {
        const std::string user = userField->getText();
        log("Network", "Username is %s", user.c_str());

        // Store config settings
        set_config_int("login", "remember", keepCheck->isMarked());
        if (keepCheck->isMarked()) {
            set_config_string("login", "username", user.c_str());
        } else {
            set_config_string("login", "username", "");
        }

        // Check login
        if (user.length() == 0) {
            ok("Error", "Enter your username first");
            warning("Enter your username first");
            state = LOGIN;
        } else {
            server_login(user, passField->getText());
            close_session();
        }
    } else if (eventId == "cancel") {
        state = EXIT;
    }
}

/*
 * Display login GUI
 */
void login() {
    // Create dialog
    dialog = new Window("Login");
    userLabel = new gcn::Label("Name:");
    passLabel = new gcn::Label("Password:");
    userField = new gcn::TextField("player");
    passField = new gcn::TextField();
    keepCheck = new CheckBox("Keep", false);
    okButton = new Button("OK");
    cancelButton = new Button("Cancel");

    dialog->setDimension(gcn::Rectangle(300, 250, 200, 80));
    userLabel->setPosition(4, 14);
    passLabel->setPosition(4, 34);
    userField->setPosition(60, 10);
    passField->setPosition(60, 30);
    userField->setWidth(130);
    passField->setWidth(130);
    keepCheck->setPosition(4, 52);
    keepCheck->setMarked(get_config_int("login", "remember", 0));
    okButton->setPosition(120, 52);
    cancelButton->setPosition(146, 52);

    userField->setEventId("ok");
    passField->setEventId("ok");
    okButton->setEventId("ok");
    cancelButton->setEventId("cancel");

    LoginActionListener *loginActionListener = new LoginActionListener();
    userField->addActionListener(loginActionListener);
    passField->addActionListener(loginActionListener);
    keepCheck->addActionListener(loginActionListener);
    okButton->addActionListener(loginActionListener);
    cancelButton->addActionListener(loginActionListener);

    dialog->add(userLabel);
    dialog->add(passLabel);
    dialog->add(userField);
    dialog->add(passField);
    dialog->add(keepCheck);
    dialog->add(okButton);
    dialog->add(cancelButton);

    guiTop->add(dialog);

    userField->requestFocus();
    userField->setCaretPosition(userField->getText().length());

    if (get_config_int("login", "remember", 0)) {
        if (get_config_string("login", "username", 0)) {
            userField->setText(get_config_string("login", "username", ""));
            passField->requestFocus();
        }
    }

    while (state == LOGIN) {
        clear_bitmap(buffer);
        blit((BITMAP *)graphic[LOGIN_BMP].dat, buffer, 0, 0, 0, 0, 800, 600);
        gui_update(NULL);
        blit(buffer, screen, 0, 0, 0, 0, 800, 600);
        if (key[KEY_ESC]) {
            state = EXIT;
        }
    }

    delete dialog;
    delete userLabel;
    delete passLabel;
    delete userField;
    delete passField;
    delete keepCheck;
    delete okButton;
    delete cancelButton;
    delete loginActionListener;
}

/** Attempt to login to login server */
void server_login(const std::string& user, const std::string& pass) {
    strncpy(username, user.c_str(), LEN_USERNAME);
    strncpy(password, pass.c_str(), LEN_PASSWORD);
    int ret;

    // Connect to login server
    ret = open_session(
            get_config_string("server", "host", 0),
            get_config_int("server", "port", 0));

    if (ret == SOCKET_ERROR) {
        state = LOGIN;
        ok("Error", "Unable to connect to login server");
        warning("Unable to connect to login server");
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
    log("Network", "Packet ID: %x", RFIFOW(0));
    log("Network", "Packet length: %d", get_packet_length(RFIFOW(0)));

    if (RFIFOW(0) == 0x0069) {
        while (in_size < RFIFOW(2)) {
            flush();
        }
        n_server = (RFIFOW(2)-47)/32;
        server_info = (SERVER_INFO*)malloc(sizeof(SERVER_INFO)*n_server);
        account_ID = RFIFOL(8);
        session_ID1 = RFIFOL(4);
        session_ID2 = RFIFOL(12);
        sex = RFIFOB(46);
        for (int i = 0; i < n_server; i++) {
            server_info[i].address = RFIFOL(47+32*i);
            memcpy(server_info[i].name, RFIFOP(47+32*i+6), 20);
            server_info[i].online_users = RFIFOW(47+32*i+26);
            server_info[i].port = RFIFOW(47+32*i+4);
            state = CHAR_SERVER;
        }
        log("Network", "Server: %s (%s:%d)", server_info[0].name,
                iptostring(server_info[0].address),
                server_info[0].port);
        log("Network", "Users: %d", server_info[0].online_users);
        RFIFOSKIP(RFIFOW(2));
    }
    else if (RFIFOW(0) == 0x006a) {
        switch (RFIFOB(2)) {
            case 0:
                ok("Error", "Unregistered ID");
                break;
            case 1:
                ok("Error", "Wrong password");
                break;
        }
        state = LOGIN;
        RFIFOSKIP(23);
    }
    else {
        ok("Error", "Unknown error");
    }
    // Todo: add other packets, also encrypted
}
