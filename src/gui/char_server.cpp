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

#include "char_server.h"
#include "../graphic/graphic.h"
#include "button.h"
#include "window.h"

char server[30];
int showServerList = 1;


ServerSelectDialog::ServerSelectDialog(gcn::Container *parent):
    Window(parent, "Select Server")
{
    serverListModel = new ServerListModel();
    serverList = new gcn::ListBox(serverListModel);
    scrollArea = new gcn::ScrollArea(serverList);
    okButton = new Button("OK");
    cancelButton = new Button("Cancel");

    setSize(200, 100);
    scrollArea->setDimension(gcn::Rectangle(4, 4, 192, 55));
    okButton->setPosition(120, 70);
    cancelButton->setPosition(146, 70);

    serverList->setEventId("ok");
    okButton->setEventId("ok");
    cancelButton->setEventId("cancel");

    serverList->addActionListener(this);
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
    ServerSelectDialog *dialog = new ServerSelectDialog(guiTop);

    state = LOGIN;

    showServerList = 1;
    while (showServerList) {
        clear_bitmap(buffer);
        blit((BITMAP *)graphic[LOGIN_BMP].dat, buffer, 0, 0, 0, 0, 800, 600);
        gui_update(NULL);
        blit(buffer, screen, 0, 0, 0, 0, 800, 600);
    }

    delete dialog;
}

void server_char_server(int serverIndex) {
    int ret;
    state = LOGIN;

    // Connect to char server
    ret = open_session(iptostring(server_info[serverIndex].address),
            server_info[serverIndex].port);
    if (ret == SOCKET_ERROR) {
        ok("Error", "Unable to connect to char server");
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

    while((in_size<4)||(out_size>0))flush();
    RFIFOSKIP(4);

    while(in_size<3)flush();

    if(RFIFOW(0)==0x006b) {
        while(in_size<RFIFOW(2))flush();
        n_character = (RFIFOW(2)-24)/106;
        char_info = (PLAYER_INFO *)malloc(sizeof(PLAYER_INFO)*n_character);
        for(int i=0;i<n_character;i++) {
            char_info[i].id = RFIFOL(24+106*i);
            strcpy(char_info[i].name, RFIFOP(24+106*i+74));
            char_info[i].hp = RFIFOW(24+106*i+42);
            char_info[i].max_hp = RFIFOW(24+106*i+44);
            char_info[i].xp = RFIFOL(24+106*i+4);
            char_info[i].gp = RFIFOL(24+106*i+8);
            char_info[i].job_xp = RFIFOL(24+106*i+12);
            char_info[i].job_lv = RFIFOL(24+106*i+16);
            char_info[i].sp = RFIFOW(24+106*i+46);
            char_info[i].max_sp = RFIFOW(24+106*i+48);
            char_info[i].lv = RFIFOW(24+106*i+58);
            char_info[i].STR = RFIFOB(24+106*i+98);
            char_info[i].AGI = RFIFOB(24+106*i+99);
            char_info[i].VIT = RFIFOB(24+106*i+100);
            char_info[i].INT = RFIFOB(24+106*i+101);
            char_info[i].DEX = RFIFOB(24+106*i+102);
            char_info[i].LUK = RFIFOB(24+106*i+103);
            char_info[i].hair_style = RFIFOW(24+106*i+54);
            char_info[i].hair_color = RFIFOW(24+106*i+70);
            char_info[i].weapon = RFIFOW(24+106*i+56);
        }
        state = CHAR_SELECT;

        log("CharServer", "Player: %s (Packet ID: %x, Length: %d",
                char_info->name, RFIFOW(0), RFIFOW(2));


        RFIFOSKIP(RFIFOW(2));
    } else if(RFIFOW(0)==0x006c) {
        switch(RFIFOB(2)) {
            case 0:
                ok("Error", "Access denied");
                break;
            case 1:
                ok("Error", "Cannot use this ID");
                break;
            default:
                ok("Error", "Rejected from server");
                break;
        }
        RFIFOSKIP(3);
        close_session();
    } else ok("Error", "Unknown error");
    // Todo: add other packets
}
