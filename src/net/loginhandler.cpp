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

#include "loginhandler.h"

#include "messagein.h"
#include "network.h"
#include "protocol.h"

#include "../log.h"
#include "../logindata.h"
#include "../main.h"
#include "../serverinfo.h"

extern SERVER_INFO **server_info;

LoginHandler::LoginHandler()
{
    static const Uint16 _messages[] = {
        SMSG_UPDATE_HOST,
        0x0069,
        0x006a,
        0
    };
    handledMessages = _messages;
}

void LoginHandler::handleMessage(MessageIn *msg)
{
    switch (msg->getId())
    {
        case 0x0063:
             int len;

             len = msg->readInt16() - 4;
             mUpdateHost = msg->readString(len);

             logger->log("Received update host \"%s\" from login server",
                          mUpdateHost.c_str());
             break;

        case 0x0069:
            // Skip the length word
            msg->skip(2);

            n_server = (msg->getLength() - 47) / 32;
            server_info =
                (SERVER_INFO**) malloc(sizeof(SERVER_INFO*) * n_server);

            mLoginData->session_ID1 = msg->readInt32();
            mLoginData->account_ID = msg->readInt32();
            mLoginData->session_ID2 = msg->readInt32();
            msg->skip(30);                           // unknown
            mLoginData->sex = msg->readInt8();

            for (int i = 0; i < n_server; i++)
            {
                server_info[i] = new SERVER_INFO;

                server_info[i]->address = msg->readInt32();
                server_info[i]->port = msg->readInt16();
                server_info[i]->name = msg->readString(20);
                server_info[i]->online_users = msg->readInt32();
                server_info[i]->updateHost = mUpdateHost;
                msg->skip(2);                        // unknown

                logger->log("Network: Server: %s (%s:%d)",
                        server_info[i]->name.c_str(),
                        iptostring(server_info[i]->address),
                        server_info[i]->port);
            }
            state = CHAR_SERVER_STATE;
            break;

        case 0x006a:
            int loginError = msg->readInt8();
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
                    errorMessage = "You have been banned from the game. Please contact the GM Team";
                    break;
                case 6:
                    errorMessage = "You have been temporarily banned from the game. Please contact the GM team";
                    break;
                case 9:
                    errorMessage = "This user name is already taken";
                    break;
                default:
                    errorMessage = "Unknown error";
                    break;
            }
            state = ERROR_STATE;
            break;
    }
}
