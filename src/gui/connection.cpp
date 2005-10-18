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

#include "connection.h"

#include <guichan/widgets/label.hpp>

#include "button.h"
#include "progressbar.h"

#include "../game.h"
#include "../log.h"
#include "../main.h"

#include "../net/messagein.h"
#include "../net/messageout.h"
#include "../net/network.h"
#include "../net/protocol.h"

ConnectionDialog::ConnectionDialog():
    Window("Info"), mProgress(0), mStatus(NET_CONNECTING)
{
    setContentSize(200, 100);
    mCancelButton = new Button("Cancel");
    mCancelButton->setPosition(5, 100 - 5 - mCancelButton->getHeight());
    mCancelButton->setEventId("cancel");
    mCancelButton->addActionListener(this);
    mProgressBar = new ProgressBar(0.0, 5, mCancelButton->getY() - 25,
                                   200 - 10, 20, 128, 128, 128);
    mLabel = new gcn::Label("Connecting...");
    mLabel->setPosition(5, mProgressBar->getY() - 25);

    add(mLabel);
    add(mCancelButton);
    add(mProgressBar);
    setLocationRelativeTo(getParent());

    const char *host = iptostring(map_address);
    openConnection(host, map_port);
}

ConnectionDialog::~ConnectionDialog()
{
}

void ConnectionDialog::logic()
{
    mProgress += 0.005f;
    if (mProgress > 1.0f)
    {
        mProgress = 0.0f;
    }
    mProgressBar->setProgress(mProgress);
    Window::logic();

    switch (mStatus)
    {
        case NET_CONNECTING:
            mStatus = pollConnection();
            break;
        case NET_ERROR:
            logger->log("Connection::Unable to connect");
            errorMessage = "Unable to connect to map server";
            state = ERROR_STATE;
            closeConnection();
            break;
        case NET_CONNECTED:
            mapLogin();
            state = GAME_STATE;
            break;
    }
}

void ConnectionDialog::action(const std::string& eventId)
{
    if (eventId == "cancel")
    {
        state = EXIT_STATE;
    }
}

void ConnectionDialog::mapLogin()
{
    // Send login infos
    MessageOut outMsg;
    outMsg.writeInt16(0x0072);
    outMsg.writeInt32(account_ID);
    outMsg.writeInt32(char_ID);
    outMsg.writeInt32(session_ID1);
    outMsg.writeInt32(session_ID2);
    outMsg.writeInt8(sex);

    // Skip a mysterious 4 bytes
    while ((in_size < 4)|| (out_size > 0)) flush();
    skip(4);

    MessageIn msg = get_next_message();

    if (msg.getId() == SMSG_LOGIN_SUCCESS)
    {
        unsigned char direction;
        msg.readInt32();   // server tick
        msg.readCoordinates(startX, startY, direction);
        msg.skip(2);      // unknown
        logger->log("Protocol: Player start position: (%d, %d), Direction: %d",
                startX, startY, direction);
    }
    else if (msg.getId() == 0x0081)
    {
        logger->log("Warning: Map server D/C");
    }
    else
    {
        logger->error("Unknown packet: map_start");
    }

    skip(msg.getLength());

    // Send "map loaded"
    // TODO: be able to reuse the same msg
    MessageOut newMsg;
    newMsg.writeInt16(0x007d);
}

void connectionInputHandler(SDL_KeyboardEvent *keyEvent)
{
    if (keyEvent->keysym.sym == SDLK_ESCAPE)
    {
        state = EXIT_STATE;
    }
}
