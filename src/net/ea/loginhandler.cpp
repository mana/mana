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

#include "net/ea/loginhandler.h"

#include "net/ea/protocol.h"

#include "net/messagein.h"
#include "net/messageout.h"

#include "log.h"
#include "logindata.h"
#include "main.h"
#include "serverinfo.h"

#include "utils/gettext.h"
#include "utils/strprintf.h"
#include "utils/stringutils.h"

extern SERVER_INFO **server_info;

Net::LoginHandler *loginHandler;

namespace EAthena {

LoginHandler::LoginHandler()
{
    static const Uint16 _messages[] = {
        SMSG_UPDATE_HOST,
        0x0069,
        0x006a,
        0
    };
    handledMessages = _messages;
    loginHandler = this;
}

void LoginHandler::handleMessage(MessageIn &msg)
{
    int code;

    switch (msg.getId())
    {
        case SMSG_UPDATE_HOST:
             int len;

             len = msg.readInt16() - 4;
             mUpdateHost = msg.readString(len);

             logger->log("Received update host \"%s\" from login server",
                     mUpdateHost.c_str());
             break;

        case 0x0069:
            // Skip the length word
            msg.skip(2);

            n_server = (msg.getLength() - 47) / 32;
            server_info =
                (SERVER_INFO**) malloc(sizeof(SERVER_INFO*) * n_server);

            mLoginData->session_ID1 = msg.readInt32();
            mLoginData->account_ID = msg.readInt32();
            mLoginData->session_ID2 = msg.readInt32();
            msg.skip(30);                           // unknown
            mLoginData->sex = msg.readInt8() ? GENDER_MALE : GENDER_FEMALE;

            for (int i = 0; i < n_server; i++)
            {
                server_info[i] = new SERVER_INFO;

                server_info[i]->address = msg.readInt32();
                server_info[i]->port = msg.readInt16();
                server_info[i]->name = msg.readString(20);
                server_info[i]->online_users = msg.readInt32();
                server_info[i]->updateHost = mUpdateHost;
                msg.skip(2);                        // unknown

                logger->log("Network: Server: %s (%s:%d)",
                        server_info[i]->name.c_str(),
                        ipToString(server_info[i]->address),
                        server_info[i]->port);
            }
            state = STATE_CHAR_SERVER;
            break;

        case 0x006a:
            code = msg.readInt8();
            logger->log("Login::error code: %i", code);

            switch (code) {
                case 0:
                    errorMessage = _("Unregistered ID");
                    break;
                case 1:
                    errorMessage = _("Wrong password");
                    break;
                case 2:
                    errorMessage = _("Account expired");
                    break;
                case 3:
                    errorMessage = _("Rejected from server");
                    break;
                case 4:

                    errorMessage = _("You have been permanently banned from "
                                     "the game. Please contact the GM Team.");
                    break;
                case 6:
                    errorMessage = strprintf(_("You have been temporarily "
                                               "banned from the game until "
                                               "%s.\n Please contact the GM "
                                               "team via the forums."),
                                               msg.readString(20).c_str());
                    break;
                case 9:
                    errorMessage = _("This user name is already taken");
                    break;
                default:
                    errorMessage = _("Unknown error");
                    break;
            }
            state = STATE_ERROR;
            break;
    }
}

void LoginHandler::loginAccount(LoginData *loginData)
{
    mLoginData = loginData;
    sendLoginRegister(loginData->username, loginData->password);
}

void LoginHandler::changeEmail(const std::string &email)
{
    // TODO
}

void LoginHandler::changePassword(const std::string &username,
                                  const std::string &oldPassword,
                                  const std::string &newPassword)
{
    // TODO
}

void LoginHandler::chooseServer(int server)
{
    // TODO
}

void LoginHandler::registerAccount(LoginData *loginData)
{
    mLoginData = loginData;

    std::string username = loginData->username;
    username.append((loginData->sex == GENDER_FEMALE) ? "_F" : "_M");

    sendLoginRegister(username, loginData->password);
}

void LoginHandler::unregisterAccount(const std::string &username,
                                     const std::string &password)
{
    // TODO
}

void LoginHandler::sendLoginRegister(const std::string &username,
                                     const std::string &password)
{
    MessageOut outMsg(0x0064);
    outMsg.writeInt32(0); // client version
    outMsg.writeString(username, 24);
    outMsg.writeString(password, 24);

    /*
     * eAthena calls the last byte "client version 2", but it isn't used at
     * at all. We're retasking it, with bit 0 to indicate whether the client
     * can handle the 0x63 "update host" packet. Clients prior to 0.0.25 send
     * 0 here.
     */
    outMsg.writeInt8(0x01);
}

} // namespace EAthena
