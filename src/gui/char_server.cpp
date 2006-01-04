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

    serverList->setEventId("ok");
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
    delete serverListModel;
}

void
ServerSelectDialog::action(const std::string& eventId)
{
    if (eventId == "ok") {
        int index = serverList->getSelected();
        const char *host = iptostring(server_info[index]->address);
        short port = server_info[index]->port;
        openConnection(host, port);
        okButton->setEnabled(false);
        //cancelButton->setEnabled(false);
        mStatus = NET_CONNECTING;
    }
    else if (eventId == "cancel") {
        state = LOGIN_STATE;
    }
}

void
ServerSelectDialog::logic()
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
            attemptServerSelect(serverList->getSelected());
            mStatus = NET_DATA;
            break;
        case NET_DATA:
            // TODO: cannot substitute with packetReady() because of eAthena
            // sending 4 unknown bytes.
            if (in_size > 6)
            {
                skip(4);
                checkServerSelect();
            }
            else
            {
                flush();
            }
            break;
    }
}

int
ServerListModel::getNumberOfElements()
{
    return n_server;
}

std::string
ServerListModel::getElementAt(int i)
{
    std::stringstream s;
    s << server_info[i]->name << " (" << server_info[i]->online_users << ")";
    return s.str();
}

void
charServerInputHandler(SDL_KeyboardEvent *keyEvent)
{
    if (keyEvent->keysym.sym == SDLK_ESCAPE)
    {
        state = LOGIN_STATE;
    }
}

void
ServerSelectDialog::attemptServerSelect(int index)
{
    // Send login infos
    MessageOut outMsg;
    outMsg.writeInt16(0x0065);
    outMsg.writeInt32(account_ID);
    outMsg.writeInt32(session_ID1);
    outMsg.writeInt32(session_ID2);
    outMsg.writeInt16(0); // unknown
    outMsg.writeInt8(sex);
}

void
ServerSelectDialog::checkServerSelect()
{
    MessageIn msg = get_next_message();

    if (msg.getId() == 0x006b)
    {
        // Skip length word and an additional mysterious 20 bytes
        msg.skip(2 + 20);

        // Derive number of characters from message length
        n_character = (msg.getLength() - 24) / 106;
        char_info = (PLAYER_INFO**)malloc(sizeof(PLAYER_INFO*) * (MAX_SLOT+1));
        for (int i = 0; i < MAX_SLOT + 1; i++)
            char_info[i] = NULL;

        for (int i = 0; i < n_character; i++)
        {
            PLAYER_INFO *tempPlayer = new PLAYER_INFO;

            tempPlayer->totalWeight = 0;
            tempPlayer->maxWeight = 0;
            tempPlayer->lastAttackTime = 0;
            tempPlayer->id = msg.readInt32();
            tempPlayer->xp = msg.readInt32();
            tempPlayer->gp = msg.readInt32();
            tempPlayer->jobXp = msg.readInt32();
            tempPlayer->jobLvl = msg.readInt32();
            msg.skip(8);                          // unknown
            msg.readInt32();                       // option
            msg.readInt32();                       // karma
            msg.readInt32();                       // manner
            msg.skip(2);                          // unknown
            tempPlayer->hp = msg.readInt16();
            tempPlayer->maxHp = msg.readInt16();
            tempPlayer->mp = msg.readInt16();
            tempPlayer->maxMp = msg.readInt16();
            msg.readInt16();                       // speed
            msg.readInt16();                       // class
            tempPlayer->hairStyle = msg.readInt16();
            tempPlayer->weapon = msg.readInt16();
            tempPlayer->lvl = msg.readInt16();
            msg.readInt16();                       // skill point
            msg.readInt16();                       // head bottom
            msg.readInt16();                       // shield
            msg.readInt16();                       // head option top
            msg.readInt16();                       // head option mid
            tempPlayer->hairColor = msg.readInt16();
            msg.readInt16();                       // unknown
            tempPlayer->name = msg.readString(24);
            tempPlayer->STR = msg.readInt8();
            tempPlayer->AGI = msg.readInt8();
            tempPlayer->VIT = msg.readInt8();
            tempPlayer->INT = msg.readInt8();
            tempPlayer->DEX = msg.readInt8();
            tempPlayer->LUK = msg.readInt8();
            int slot = msg.readInt8();  // character slot
            msg.readInt8();                        // unknown
            
            char_info[slot] = tempPlayer;
            
            logger->log("CharServer: Player: %s (%d)",
                        char_info[slot]->name.c_str(), slot);
        }

        state = CHAR_SELECT_STATE;
        skip(msg.getLength());
    }
    else if (msg.getId() == 0x006c)
    {
        std::string errorStr;
        switch (msg.readInt8()) {
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
