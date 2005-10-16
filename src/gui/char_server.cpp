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

#include <sstream>
#include <SDL.h>

#include "button.h"
#include "listbox.h"
#include "ok_dialog.h"
#include "scrollarea.h"

#include "../log.h"
#include "../main.h"
#include "../playerinfo.h"
#include "../serverinfo.h"

#include "../net/messagein.h"
#include "../net/messageout.h"
#include "../net/network.h"

extern SERVER_INFO **server_info;

char server[30];


ServerSelectDialog::ServerSelectDialog():
    Window("Select Server"), mStatus(NET_IDLE)
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
        okButton->setEnabled(false);
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
        int index = serverList->getSelected();
        const char *host = iptostring(server_info[index]->address);
        short port = server_info[index]->port;
        openConnection(host, port);
        mStatus = NET_CONNECTING;
        //server_char_server(serverList->getSelected());
    }
    else if (eventId == "cancel") {
        state = LOGIN_STATE;
    }
}

void ServerSelectDialog::logic()
{
    switch (mStatus)
    {
        case NET_CONNECTING:
            mStatus = pollConnection();
            break;
        case NET_ERROR:
            logger->log("ServerSelect::Unable to connect");
            errorMessage = "Unable to connect to char server";
            state = ERROR_STATE;
            closeConnection();
            break;
        case NET_CONNECTED:
            selectServer(serverList->getSelected());
            //closeConnection();
            break;
    }
}

int ServerListModel::getNumberOfElements()
{
    return n_server;
}

std::string ServerListModel::getElementAt(int i)
{
    std::stringstream s;
    s << server_info[i]->name << " (" << server_info[i]->online_users << ")";
    return s.str();
}

void charServerInputHandler(SDL_KeyboardEvent *keyEvent)
{
    if (keyEvent->keysym.sym == SDLK_ESCAPE)
    {
        state = LOGIN_STATE;
    }
}

void ServerSelectDialog::selectServer(int index)
{
    // Send login infos
    MessageOut outMsg;
    outMsg.writeShort(0x0065);
    outMsg.writeLong(account_ID);
    outMsg.writeLong(session_ID1);
    outMsg.writeLong(session_ID2);
    outMsg.writeShort(0); // unknown
    outMsg.writeByte(sex);

    // Skipping a mysterious 4 bytes
    while ((in_size < 4) || (out_size > 0)) flush();
    skip(4);

    MessageIn msg = get_next_message();

    if (msg.getId() == 0x006b)
    {
        // Skip length word and an additional mysterious 20 bytes
        msg.skip(2 + 20);

        // Derive number of characters from message length
        n_character = (msg.getLength() - 24) / 106;
        char_info = (PLAYER_INFO**)malloc(sizeof(PLAYER_INFO*) * n_character);

        for (int i = 0; i < n_character; i++)
        {
            char_info[i] = new PLAYER_INFO;

            char_info[i]->totalWeight = 0;
            char_info[i]->maxWeight = 0;
            char_info[i]->lastAttackTime = 0;
            char_info[i]->id = msg.readLong();
            char_info[i]->xp = msg.readLong();
            char_info[i]->gp = msg.readLong();
            char_info[i]->jobXp = msg.readLong();
            char_info[i]->jobLvl = msg.readLong();
            msg.skip(8);                          // unknown
            msg.readLong();                       // option
            msg.readLong();                       // karma
            msg.readLong();                       // manner
            msg.skip(2);                          // unknown
            char_info[i]->hp = msg.readShort();
            char_info[i]->maxHp = msg.readShort();
            char_info[i]->mp = msg.readShort();
            char_info[i]->maxMp = msg.readShort();
            msg.readShort();                       // speed
            msg.readShort();                       // class
            char_info[i]->hairStyle = msg.readShort();
            char_info[i]->weapon = msg.readShort();
            char_info[i]->lvl = msg.readShort();
            msg.readShort();                       // skill point
            msg.readShort();                       // head bottom
            msg.readShort();                       // shield
            msg.readShort();                       // head option top
            msg.readShort();                       // head option mid
            char_info[i]->hairColor = msg.readShort();
            msg.readShort();                       // unknown
            char_info[i]->name = msg.readString(24);
            char_info[i]->STR = msg.readByte();
            char_info[i]->AGI = msg.readByte();
            char_info[i]->VIT = msg.readByte();
            char_info[i]->INT = msg.readByte();
            char_info[i]->DEX = msg.readByte();
            char_info[i]->LUK = msg.readByte();
            char_info[i]->characterNumber = msg.readByte();  // character number
            msg.readByte();                        // unknown
        }

        state = CHAR_SELECT_STATE;

        logger->log("CharServer: Player: %s (Packet ID: %x, Length: %d)",
                    char_info[0]->name.c_str(), msg.getId(), msg.getLength());

        skip(msg.getLength());
    }
    else if (msg.getId() == 0x006c)
    {
        std::string errorStr;
        switch (msg.readByte()) {
            case 0: errorStr = "Access denied"; break;
            case 1: errorStr = "Cannot use this ID"; break;
            default: errorStr = "Rejected from server"; break;
        }
        new OkDialog("Error", errorStr);
        skip(msg.getLength());
        closeConnection();
    }
    else
    {
        new OkDialog("Error", "Unknown error");
        skip(msg.getLength());
    }
    // Todo: add other packets
}
