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

#include "logouthandler.h"

#include "messagein.h"
#include "protocol.h"

#include "../main.h"

LogoutHandler::LogoutHandler():
mPassToken(NULL), mScenario(LOGOUT_EXIT),
mLoggedOutAccount(false), mLoggedOutGame(false), mLoggedOutChat(false)
{
    static const Uint16 _messages[] = {
        APMSG_LOGOUT_RESPONSE,
        APMSG_UNREGISTER_RESPONSE,
        GPMSG_DISCONNECT_RESPONSE,
        CPMSG_DISCONNECT_RESPONSE,
        0
    };
    handledMessages = _messages;
}

void LogoutHandler::handleMessage(MessageIn &msg)
{
    switch (msg.getId())
    {
        case APMSG_LOGOUT_RESPONSE:
        {
            int errMsg = msg.readInt8();

            // Successful logout
            if (errMsg == ERRMSG_OK)
            {
                mLoggedOutAccount = true;

                switch (mScenario)
                {
                    case LOGOUT_SWITCH_ACCOUNTSERVER:
                        if (mLoggedOutGame && mLoggedOutChat)
                                           state = STATE_SWITCH_ACCOUNTSERVER;
                        break;

                    case LOGOUT_EXIT:
                    default:
                        if (mLoggedOutGame && mLoggedOutChat)
                                                     state = STATE_FORCE_QUIT;
                        break;
                }
            }
            // Logout failed
            else
            {
                switch (errMsg) {
                    case ERRMSG_NO_LOGIN:
                        errorMessage = "Accountserver: Not logged in";
                        break;
                    default:
                        errorMessage = "Accountserver: Unknown error";
                        break;
                }
                state = STATE_ERROR;
            }
        }
            break;
        case APMSG_UNREGISTER_RESPONSE:
        {
            int errMsg = msg.readInt8();
            // Successful unregistration
            if (errMsg == ERRMSG_OK)
            {
                state = STATE_UNREGISTER;
            }
            // Unregistration failed
            else
            {
                switch (errMsg) {
                    case ERRMSG_INVALID_ARGUMENT:
                        errorMessage =
                                  "Accountserver: Wrong username or password";
                        break;
                    default:
                        errorMessage = "Accountserver: Unknown error";
                        break;
                }
                state = STATE_ACCOUNTCHANGE_ERROR;
            }
        }
            break;
        case GPMSG_DISCONNECT_RESPONSE:
        {
            int errMsg = msg.readInt8();
            // Successful logout
            if (errMsg == ERRMSG_OK)
            {
                mLoggedOutGame = true;

                switch (mScenario)
                {
                    case LOGOUT_SWITCH_CHARACTER:
                        if (mPassToken)
                        {
                            *mPassToken = msg.readString(32);
                            mPassToken = NULL;
                        }
                        if (mLoggedOutChat) state = STATE_RECONNECT_ACCOUNT;
                        break;

                    case LOGOUT_SWITCH_ACCOUNTSERVER:
                        if (mLoggedOutAccount && mLoggedOutChat)
                                           state = STATE_SWITCH_ACCOUNTSERVER;
                        break;

                    case LOGOUT_EXIT:
                    default:
                        if (mLoggedOutAccount && mLoggedOutChat)
                                                     state = STATE_FORCE_QUIT;
                        break;
                }
            }
            // Logout failed
            else
            {
                switch (errMsg) {
                    case ERRMSG_NO_LOGIN:
                        errorMessage = "Gameserver: Not logged in";
                        break;
                    default:
                        errorMessage = "Gameserver: Unknown error";
                        break;
                }
                state = STATE_ERROR;
            }
        }
            break;
        case CPMSG_DISCONNECT_RESPONSE:
        {
            int errMsg = msg.readInt8();
            // Successful logout
            if (errMsg == ERRMSG_OK)
            {
                mLoggedOutChat = true;

                switch (mScenario)
                {
                    case LOGOUT_SWITCH_CHARACTER:
                        if (mLoggedOutGame) state = STATE_RECONNECT_ACCOUNT;
                        break;

                    case LOGOUT_SWITCH_ACCOUNTSERVER:
                        if (mLoggedOutAccount && mLoggedOutGame)
                                          state = STATE_SWITCH_ACCOUNTSERVER;
                        break;

                    case LOGOUT_EXIT:
                    default:
                        if (mLoggedOutAccount && mLoggedOutGame)
                        {
                            state = STATE_FORCE_QUIT;
                        }
                        break;
                }
            }
            else
            {
                switch (errMsg) {
                    case ERRMSG_NO_LOGIN:
                        errorMessage = "Chatserver: Not logged in";
                        break;
                    default:
                        errorMessage = "Chatserver: Unknown error";
                        break;
                }
                state = STATE_ERROR;
            }
        }
            break;
    }
}

void
LogoutHandler::setScenario(unsigned short scenario, std::string* passToken)
{
    mScenario = scenario;
    mPassToken = passToken;
}

void
LogoutHandler::reset()
{
    mPassToken = NULL;
    mScenario = LOGOUT_EXIT;
    mLoggedOutAccount = false;
    mLoggedOutGame = false;
    mLoggedOutChat = false;
}
