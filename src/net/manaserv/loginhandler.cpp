/*
 *  The Mana World
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#include "net/manaserv/loginhandler.h"

#include "main.h"

#include "net/logindata.h"

#include "net/manaserv/connection.h"
#include "net/manaserv/messagein.h"
#include "net/manaserv/messageout.h"
#include "net/manaserv/protocol.h"

#include "utils/gettext.h"
#include "utils/sha256.h"

extern Net::LoginHandler *loginHandler;

namespace ManaServ {

extern Connection *accountServerConnection;
extern std::string netToken;

LoginHandler::LoginHandler()
{
    static const Uint16 _messages[] = {
        APMSG_LOGIN_RESPONSE,
        APMSG_REGISTER_RESPONSE,
        APMSG_RECONNECT_RESPONSE,
        APMSG_PASSWORD_CHANGE_RESPONSE,
        APMSG_EMAIL_CHANGE_RESPONSE,
        APMSG_LOGOUT_RESPONSE,
        APMSG_UNREGISTER_RESPONSE,
        APMSG_REGISTER_INFO_RESPONSE,
        0
    };
    handledMessages = _messages;
    loginHandler = this;
}

void LoginHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case APMSG_LOGIN_RESPONSE:
            handleLoginResponse(msg);
            break;

        case APMSG_REGISTER_RESPONSE:
            handleRegisterResponse(msg);
            break;

        case APMSG_RECONNECT_RESPONSE:
        {
            int errMsg = msg.readInt8();
            // Successful login
            if (errMsg == ERRMSG_OK)
            {
                state = STATE_CHAR_SELECT;
            }
            // Login failed
            else
            {
                switch (errMsg)
                {
                    case ERRMSG_INVALID_ARGUMENT:
                        errorMessage = _("Wrong magic_token.");
                        break;
                    case ERRMSG_FAILURE:
                        errorMessage = _("Already logged in.");
                        break;
                    case LOGIN_SERVER_FULL:
                        errorMessage = _("Server is full.");
                        break;
                    default:
                        errorMessage = _("Unknown error.");
                        break;
                }
                state = STATE_ERROR;
            }
        }
            break;

        case APMSG_PASSWORD_CHANGE_RESPONSE:
        {
            int errMsg = msg.readInt8();
            // Successful pass change
            if (errMsg == ERRMSG_OK)
            {
                state = STATE_CHANGEPASSWORD_SUCCESS;
            }
            // pass change failed
            else
            {
                switch (errMsg)
                {
                    case ERRMSG_INVALID_ARGUMENT:
                        errorMessage = _("New password incorrect.");
                        break;
                    case ERRMSG_FAILURE:
                        errorMessage = _("Old password incorrect.");
                        break;
                    case ERRMSG_NO_LOGIN:
                        errorMessage = _("Account not connected. Please login first.");
                        break;
                    default:
                        errorMessage = _("Unknown error.");
                        break;
                }
                state = STATE_ACCOUNTCHANGE_ERROR;
            }
        }
            break;

        case APMSG_EMAIL_CHANGE_RESPONSE:
        {
            int errMsg = msg.readInt8();
            // Successful pass change
            if (errMsg == ERRMSG_OK)
            {
                state = STATE_CHANGEEMAIL_SUCCESS;
            }
            // pass change failed
            else
            {
                switch (errMsg)
                {
                    case ERRMSG_INVALID_ARGUMENT:
                        errorMessage = _("New email address incorrect.");
                        break;
                    case ERRMSG_FAILURE:
                        errorMessage = _("Old email address incorrect.");
                        break;
                    case ERRMSG_NO_LOGIN:
                        errorMessage = _("Account not connected. Please login first.");
                        break;
                    case ERRMSG_EMAIL_ALREADY_EXISTS:
                        errorMessage = _("The new email address already exists.");
                        break;
                    default:
                        errorMessage = _("Unknown error.");
                        break;
                }
                state = STATE_ACCOUNTCHANGE_ERROR;
            }
        }
            break;
        case APMSG_LOGOUT_RESPONSE:
        {
            int errMsg = msg.readInt8();

            // Successful logout
            if (errMsg == ERRMSG_OK)
            {
                // TODO: handle logout
            }
            // Logout failed
            else
            {
                switch (errMsg)
                {
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
                switch (errMsg)
                {
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

        case APMSG_REGISTER_INFO_RESPONSE:
        {
            int allowed = msg.readInt8();

            if (allowed)
            {
                mMinUserNameLength = msg.readInt8();
                mMaxUserNameLength = msg.readInt8();
                std::string captchaURL = msg.readString();
                std::string captchaInstructions = msg.readString();

                printf("%s: %s\n", captchaURL.c_str(), captchaInstructions.c_str());

                state = STATE_REGISTER;
            }
            else
            {
                errorMessage = msg.readString();

                if (errorMessage.empty())
                    errorMessage = _("Client registration is not allowed. "
                                     "Please contact server administration.");
                state = STATE_LOGIN_ERROR;
            }
        }
            break;
    }
}

void LoginHandler::handleLoginResponse(Net::MessageIn &msg)
{
    const int errMsg = msg.readInt8();

    if (errMsg == ERRMSG_OK)
    {
        readUpdateHost(msg);
        // No worlds atm, but future use :-D
        state = STATE_WORLD_SELECT;
    }
    else
    {
        switch (errMsg)
        {
            case LOGIN_INVALID_VERSION:
                errorMessage = _("Client version is too old.");
                break;
            case ERRMSG_INVALID_ARGUMENT:
                errorMessage = _("Wrong username or password.");
                break;
            case ERRMSG_FAILURE:
                errorMessage = _("Already logged in.");
                break;
            case LOGIN_SERVER_FULL:
                errorMessage = _("Server is full.");
                break;
            case LOGIN_INVALID_TIME:
                errorMessage = _("Login attempt too soon after previous "
                                 "attempt.");
                break;
            default:
                errorMessage = _("Unknown error.");
                break;
        }
        state = STATE_LOGIN_ERROR;
    }
}

void LoginHandler::handleRegisterResponse(Net::MessageIn &msg)
{
    const int errMsg = msg.readInt8();

    if (errMsg == ERRMSG_OK)
    {
        readUpdateHost(msg);
        state = STATE_WORLD_SELECT;
    }
    else
    {
        switch (errMsg)
        {
            case REGISTER_INVALID_VERSION:
                errorMessage = _("Client version is too old.");
                break;
            case ERRMSG_INVALID_ARGUMENT:
                errorMessage = _("Wrong username, password or email address.");
                break;
            case REGISTER_EXISTS_USERNAME:
                errorMessage = _("Username already exists.");
                break;
            case REGISTER_EXISTS_EMAIL:
                errorMessage = _("Email address already exists.");
                break;
            case REGISTER_CAPTCHA_WRONG:
                errorMessage = _("You took too long with the captcha or your "
                                 "response was incorrect.");
                break;
            default:
                errorMessage = _("Unknown error.");
                break;
        }
        state = STATE_LOGIN_ERROR;
    }
}

void LoginHandler::readUpdateHost(Net::MessageIn &msg)
{
    // Set the update host when included in the message
    if (msg.getUnreadLength() > 0)
    {
        mLoginData->updateHost = msg.readString();
    }
}

void LoginHandler::connect()
{
    accountServerConnection->connect(mServer.hostname, mServer.port);
}

bool LoginHandler::isConnected()
{
    return accountServerConnection->isConnected();
}

void LoginHandler::disconnect()
{
    accountServerConnection->disconnect();

    if (state == STATE_CONNECT_GAME)
    {
        state = STATE_GAME;
    }
}

void LoginHandler::getRegistrationDetails()
{
    MessageOut msg(PAMSG_REQUEST_REGISTER_INFO);
    accountServerConnection->send(msg);
}

unsigned int LoginHandler::getMinUserNameLength() const
{
    return mMinUserNameLength;
}

unsigned int LoginHandler::getMaxUserNameLength() const
{
    return mMaxUserNameLength;
}

void LoginHandler::loginAccount(LoginData *loginData)
{
    mLoginData = loginData;

    MessageOut msg(PAMSG_LOGIN);

    msg.writeInt32(0); // client version
    msg.writeString(loginData->username);
    msg.writeString(sha256(loginData->username + loginData->password));

    accountServerConnection->send(msg);
}

void LoginHandler::logout()
{
    MessageOut msg(PAMSG_LOGOUT);
    accountServerConnection->send(msg);
}

void LoginHandler::changeEmail(const std::string &email)
{
    MessageOut msg(PAMSG_EMAIL_CHANGE);

    // Email is sent clearly so the server can validate the data.
    // Encryption is assumed server-side.
    msg.writeString(email);

    accountServerConnection->send(msg);
}

void LoginHandler::changePassword(const std::string &username,
                    const std::string &oldPassword,
                    const std::string &newPassword)
{
    MessageOut msg(PAMSG_PASSWORD_CHANGE);

    // Change password using SHA2 encryption
    msg.writeString(sha256(username + oldPassword));
    msg.writeString(sha256(username + newPassword));

    accountServerConnection->send(msg);
}

void LoginHandler::chooseServer(unsigned int server)
{
    // TODO
}

void LoginHandler::registerAccount(LoginData *loginData)
{
    MessageOut msg(PAMSG_REGISTER);

    msg.writeInt32(0); // client version
    msg.writeString(loginData->username);
    // Use a hashed password for privacy reasons
    msg.writeString(sha256(loginData->username + loginData->password));
    msg.writeString(loginData->email);
    msg.writeString(loginData->captchaResponse);

    accountServerConnection->send(msg);
}

void LoginHandler::unregisterAccount(const std::string &username,
                        const std::string &password)
{
    MessageOut msg(PAMSG_UNREGISTER);

    msg.writeString(username);
    msg.writeString(sha256(username + password));

    accountServerConnection->send(msg);
}

Worlds LoginHandler::getWorlds() const
{
    return Worlds();
}

void LoginHandler::reconnect()
{
    MessageOut msg(PAMSG_RECONNECT);
    msg.writeString(netToken, 32);
    accountServerConnection->send(msg);
}

} // namespace ManaServ
