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

#include "char_server.h"

#include <guichan/sdl/sdlinput.hpp>

#include "button.h"
#include "gui.h"
#include "listbox.h"
#include "ok_dialog.h"
#include "scrollarea.h"

#include "../graphics.h"
#include "../log.h"
#include "../main.h"
#include "../playerinfo.h"
#include "../serverinfo.h"

#include "../net/network.h"

extern Graphics *graphics;

char server[30];
int showServerList = 1;


ServerSelectDialog::ServerSelectDialog():
    Window("Select Server")
{
    serverListModel = new ServerListModel();
    serverList = new ListBox(serverListModel);
    scrollArea = new ScrollArea(serverList);
    okButton = new Button("OK");
    cancelButton = new Button("Cancel");

    setContentSize(200, 100);

    cancelButton->setPosition(
            200 - cancelButton->getWidth() - 5,
            100 - cancelButton->getHeight() - 5);
    okButton->setPosition(
            cancelButton->getX() - okButton->getWidth() - 5,
            100 - okButton->getHeight() - 5);
    scrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    scrollArea->setDimension(gcn::Rectangle(
                5, 5, 200 - 2 * 5,
                100 - 3 * 5 - cancelButton->getHeight() -
                scrollArea->getBorderSize()));

    //serverList->setEventId("ok");
    okButton->setEventId("ok");
    cancelButton->setEventId("cancel");

    //serverList->addActionListener(this);
    okButton->addActionListener(this);
    cancelButton->addActionListener(this);

    add(scrollArea);
    add(okButton);
    add(cancelButton);

    if (n_server == 0) {
        // Disable Ok button
        //okButton->char_server_dialog[2].flags |= D_DISABLED;
    } else {
        // Select first server
        serverList->setSelected(1);
    }

    okButton->requestFocus();
    setLocationRelativeTo(getParent());
}

ServerSelectDialog::~ServerSelectDialog()
{
    delete serverList;
    delete serverListModel;
    delete scrollArea;
    delete okButton;
    delete cancelButton;
}

void ServerSelectDialog::action(const std::string& eventId)
{
    if (eventId == "ok") {
        server_char_server(serverList->getSelected());
    }
    else if (eventId == "cancel") {
    }
    showServerList = 0;
}


int ServerListModel::getNumberOfElements() {
    return n_server;
}

std::string ServerListModel::getElementAt(int i) {
    static char buffer[30];
    sprintf(buffer, "%s (%i)", server_info[i].name,
            server_info[i].online_users);
    return buffer;
}


void char_server() {
    ServerSelectDialog *dialog = new ServerSelectDialog();

    state = LOGIN;

    showServerList = 1;
    while (showServerList)
    {
        // Handle SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    state = EXIT;
                    showServerList = false;
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        showServerList = false;
                    }
                    break;
            }

            guiInput->pushInput(event);
        }

        gui->logic();

        graphics->drawImage(login_wallpaper, 0, 0);
        gui->draw();
        graphics->updateScreen();
    }

    delete dialog;
}

void server_char_server(int serverIndex) {
    int ret;
    state = LOGIN;
    const char *ipstring = iptostring(server_info[serverIndex].address);

    // Connect to char server
    ret = open_session(ipstring, server_info[serverIndex].port);
    if (ret == SOCKET_ERROR) {
        std::string str = std::string("Unable to connect to char server ") +
            std::string(ipstring);
        new OkDialog("Error", str);
        return;
    }

    // Send login infos
    WFIFOW(0) = net_w_value(0x0065);
    WFIFOL(2) = net_l_value(account_ID);
    WFIFOL(6) = net_l_value(session_ID1);
    WFIFOL(10) = net_l_value(session_ID2);
    WFIFOW(14) = 0;
    WFIFOB(16) = net_b_value(sex);
    WFIFOSET(17);

    while ((in_size < 4) || (out_size > 0))flush();
    RFIFOSKIP(4);

    while (in_size < 3) flush();

    if (RFIFOW(0) == 0x006b) {
        while(in_size < RFIFOW(2))flush();
        n_character = (RFIFOW(2) - 24) / 106;
        char_info = (PLAYER_INFO *)malloc(sizeof(PLAYER_INFO) * n_character);
        for (int i = 0; i < n_character; i++) {
            int n = 24 + 106 * i;
            char_info[i].id = RFIFOL(n);
            strcpy(char_info[i].name, RFIFOP(n + 74));
            char_info[i].hp = RFIFOW( n+ 42);
            char_info[i].max_hp = RFIFOW(n + 44);
            char_info[i].xp = RFIFOL(n + 4);
            char_info[i].gp = RFIFOL(n + 8);
            char_info[i].job_xp = RFIFOL(n + 12);
            char_info[i].job_lv = RFIFOL(n + 16);
            char_info[i].sp = RFIFOW(n + 46);
            char_info[i].max_sp = RFIFOW(n + 48);
            char_info[i].lv = RFIFOW(n + 58);
            char_info[i].STR = RFIFOB(n + 98);
            char_info[i].AGI = RFIFOB(n + 99);
            char_info[i].VIT = RFIFOB(n + 100);
            char_info[i].INT = RFIFOB(n + 101);
            char_info[i].DEX = RFIFOB(n + 102);
            char_info[i].LUK = RFIFOB(n + 103);
            char_info[i].hair_style = RFIFOW(n + 54);
            char_info[i].hair_color = RFIFOW(n + 70);
            char_info[i].weapon = RFIFOW(n + 56);
        }
        state = CHAR_SELECT;

        logger->log("CharServer: Player: %s (Packet ID: %x, Length: %d)",
                char_info->name, RFIFOW(0), RFIFOW(2));


        RFIFOSKIP(RFIFOW(2));
    }
    else if (RFIFOW(0) == 0x006c) {
        std::string errorStr;
        switch (RFIFOB(2)) {
            case 0: errorStr = "Access denied"; break;
            case 1: errorStr = "Cannot use this ID"; break;
            default: errorStr = "Rejected from server"; break;
        }
        new OkDialog("Error", errorStr);
        RFIFOSKIP(3);
        close_session();
    } else {
        new OkDialog("Error", "Unknown error");
    }
    // Todo: add other packets
}
