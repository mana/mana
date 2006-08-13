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

#include "../localplayer.h"
#include "../log.h"
#include "../logindata.h"
#include "../main.h"

LoginHandler::LoginHandler()
{
    static const Uint16 _messages[] = {
        APMSG_LOGIN_RESPONSE,
        APMSG_REGISTER_RESPONSE,
        0
    };
    handledMessages = _messages;
}

void LoginHandler::handleMessage(MessageIn *msg)
{
    switch (msg->getId())
    {
        case APMSG_LOGIN_RESPONSE:
        {
            int errMsg = msg->readByte();
            // Successful login
            if (errMsg == ERRMSG_OK)
            {
                unsigned char charNumber = msg->readByte();
                printf("Account has %i characters:\n", charNumber);
                for (unsigned int i = 0; i < charNumber; i++) {
                    // Create a temp empty player to show up in character
                    // selection dialog
                    LocalPlayer *temp = new LocalPlayer(0, 0, 0);
                    temp->setName(msg->readString());
                    temp->setSex(msg->readByte());
                    temp->setHairStyle(msg->readByte());
                    temp->setHairColor(msg->readByte());
                    temp->mLevel = msg->readByte();
                    temp->mGp = msg->readShort();
                    mCharInfo->select(i);
                    mCharInfo->setEntry(temp);
                }
                state = CHAR_SELECT_STATE;
            }
            // Login failed
            else
            {
                switch (errMsg) {
                    case LOGIN_INVALID_VERSION:
                    errorMessage = "Client has an insufficient version number to login.";
                        break;
                    case ERRMSG_INVALID_ARGUMENT:
                        errorMessage = "Wrong username or password";
                        break;
                    case ERRMSG_FAILURE:
                        errorMessage = "Already logged in";
                        break;
                    case LOGIN_SERVER_FULL:
                        errorMessage = "Server is full";
                        break;
                    default:
                        errorMessage = "Unknown error";
                        break;
                }
                state = ERROR_STATE;
            }
        }
            break;
        case APMSG_REGISTER_RESPONSE:
        {
            int errMsg = msg->readByte();
            // Successful registration
            if (errMsg == ERRMSG_OK)
            {
                state = ACCOUNT_STATE;
            }
            // Registration failed
            else {
                switch (errMsg) {
                    case REGISTER_INVALID_VERSION:
                    errorMessage = "Client has an insufficient version number to login.";
                        break;
                    case ERRMSG_INVALID_ARGUMENT:
                        errorMessage = "Wrong username, password or email address";
                        break;
                    case REGISTER_EXISTS_USERNAME:
                        errorMessage = "Username already exists";
                        break;
                    case REGISTER_EXISTS_EMAIL:
                        errorMessage = "Email address already exists";
                        break;
                    default:
                        errorMessage = "Unknown error";
                        break;
                }
                state = ERROR_STATE;
            }
        }
            break;
    }
}
