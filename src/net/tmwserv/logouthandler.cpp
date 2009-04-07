/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "net/tmwserv/logouthandler.h"

#include "net/tmwserv/connection.h"
#include "net/tmwserv/protocol.h"

#include "net/tmwserv/accountserver/accountserver.h"
#include "net/tmwserv/chatserver/chatserver.h"
#include "net/tmwserv/gameserver/gameserver.h"

#include "net/messagein.h"

#include "main.h"

Net::LogoutHandler *logoutHandler;

extern Net::Connection *gameServerConnection;
extern Net::Connection *chatServerConnection;
extern Net::Connection *accountServerConnection;

namespace TmwServ {

LogoutHandler::LogoutHandler():
    mPassToken(NULL),
    mScenario(LOGOUT_EXIT),
    mLoggedOutAccount(false),
    mLoggedOutGame(false),
    mLoggedOutChat(false)
{
    static const Uint16 _messages[] = {
        APMSG_LOGOUT_RESPONSE,
        APMSG_UNREGISTER_RESPONSE,
        GPMSG_DISCONNECT_RESPONSE,
        CPMSG_DISCONNECT_RESPONSE,
        0
    };
    handledMessages = _messages;
    logoutHandler = this;
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
                    case LOGOUT_SWITCH_LOGIN:
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

                    case LOGOUT_SWITCH_LOGIN:
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

                    case LOGOUT_SWITCH_LOGIN:
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

void LogoutHandler::setScenario(unsigned short scenario,
                                std::string *passToken)
{
    mScenario = scenario;
    mPassToken = passToken;

    // Can't logout if we were not logged in ...
    if (mScenario == LOGOUT_EXIT)
    {
        if (accountServerConnection->isConnected())
            Net::AccountServer::logout();
        else
            setAccountLoggedOut();

        if (gameServerConnection->isConnected())
            Net::GameServer::logout(false);
        else
            setGameLoggedOut();

        if (chatServerConnection->isConnected())
            Net::ChatServer::logout();
        else
            setChatLoggedOut();
    }
    else if (mScenario == LOGOUT_SWITCH_LOGIN)
    {
        if (accountServerConnection->isConnected())
            Net::AccountServer::logout();
        else
            setAccountLoggedOut();

        if (gameServerConnection->isConnected())
            Net::GameServer::logout(false);
        else
            setGameLoggedOut();

        if (chatServerConnection->isConnected())
            Net::ChatServer::logout();
        else
            setChatLoggedOut();
    }
    else if (mScenario == LOGOUT_SWITCH_CHARACTER)
    {
        Net::GameServer::logout(true);
        Net::ChatServer::logout();
    }
}

void LogoutHandler::reset()
{
    mPassToken = NULL;
    mScenario = LOGOUT_EXIT;
    mLoggedOutAccount = false;
    mLoggedOutGame = false;
    mLoggedOutChat = false;
}

} // namespace TmwServ
