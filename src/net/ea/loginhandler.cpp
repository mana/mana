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

#include "net/ea/network.h"
#include "net/ea/protocol.h"

#include "net/logindata.h"
#include "net/messagein.h"
#include "net/messageout.h"

#include "log.h"
#include "main.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

extern Net::LoginHandler *loginHandler;

namespace EAthena {

extern ServerInfo charServer;

LoginHandler::LoginHandler()
{
    static const Uint16 _messages[] = {
        SMSG_UPDATE_HOST,
        SMSG_LOGIN_DATA,
        SMSG_LOGIN_ERROR,
        SMSG_CHAR_PASSWORD_RESPONSE,
        SMSG_SERVER_VERSION_RESPONSE,
        0
    };
    handledMessages = _messages;
    loginHandler = this;
}

void LoginHandler::handleMessage(Net::MessageIn &msg)
{
    int code, worldCount;

    switch (msg.getId())
    {
        case SMSG_CHAR_PASSWORD_RESPONSE:
        {
            // 0: acc not found, 1: success, 2: password mismatch, 3: pass too short
            int errMsg = msg.readInt8();
            // Successful pass change
            if (errMsg == 1)
            {
                state = STATE_CHANGEPASSWORD_SUCCESS;
            }
            // pass change failed
            else
            {
                switch (errMsg) {
                    case 0:
                        errorMessage = _("Account was not found. Please re-login.");
                        break;
                    case 2:
                        errorMessage = _("Old password incorrect.");
                        break;
                    case 3:
                        errorMessage = _("New password too short.");
                        break;
                    default:
                        errorMessage = _("Unknown error.");
                        break;
                }
                state = STATE_ACCOUNTCHANGE_ERROR;
            }
        }
            break;

        case SMSG_UPDATE_HOST:
             int len;

             len = msg.readInt16() - 4;
             mUpdateHost = msg.readString(len);
             loginData.updateHost = mUpdateHost;

             logger->log("Received update host \"%s\" from login server.",
                     mUpdateHost.c_str());
             break;

        case SMSG_LOGIN_DATA:
            // Skip the length word
            msg.skip(2);

            clearWorlds();

            worldCount = (msg.getLength() - 47) / 32;

            mToken.session_ID1 = msg.readInt32();
            mToken.account_ID = msg.readInt32();
            mToken.session_ID2 = msg.readInt32();
            msg.skip(30);                           // unknown
            mToken.sex = msg.readInt8() ? GENDER_MALE : GENDER_FEMALE;

            for (int i = 0; i < worldCount; i++)
            {
                WorldInfo *world = new WorldInfo;

                world->address = msg.readInt32();
                world->port = msg.readInt16();
                world->name = msg.readString(20);
                world->online_users = msg.readInt32();
                world->updateHost = mUpdateHost;
                msg.skip(2);                        // unknown

                logger->log("Network: Server: %s (%s:%d)",
                        world->name.c_str(),
                        ipToString(world->address),
                        world->port);

                mWorlds.push_back(world);
            }
            state = STATE_WORLD_SELECT;
            break;

        case SMSG_LOGIN_ERROR:
            code = msg.readInt8();
            logger->log("Login::error code: %i", code);

            switch (code) {
                case 0:
                    errorMessage = _("Unregistered ID.");
                    break;
                case 1:
                    errorMessage = _("Wrong password.");
                    break;
                case 2:
                    errorMessage = _("Account expired.");
                    break;
                case 3:
                    errorMessage = _("Rejected from server.");
                    break;
                case 4:
                    errorMessage = _("You have been permanently banned from "
                                     "the game. Please contact the GM team.");
                    break;
                case 6:
                    errorMessage = strprintf(_("You have been temporarily "
                                               "banned from the game until "
                                               "%s.\nPlease contact the GM "
                                               "team via the forums."),
                                               msg.readString(20).c_str());
                    break;
                case 9:
                    errorMessage = _("This user name is already taken.");
                    break;
                default:
                    errorMessage = _("Unknown error.");
                    break;
            }
            state = STATE_ERROR;
            break;
        case SMSG_SERVER_VERSION_RESPONSE:
            {
                // TODO: verify these!
                msg.readInt8(); // -1
                msg.readInt8(); // T
                msg.readInt8(); // M
                msg.readInt8(); // W
                msg.readInt8(); // (space)
                msg.readInt8(); // e
                msg.readInt8(); // A

                //state = STATE_LOGIN;
            }
            break;
    }
}

void LoginHandler::connect()
{
    mNetwork->connect(mServer);
    MessageOut outMsg(CMSG_SERVER_VERSION_REQUEST);
}

bool LoginHandler::isConnected()
{
    return mNetwork->isConnected();
}

void LoginHandler::disconnect()
{
    if (mNetwork->getServer() == mServer)
        mNetwork->disconnect();
}

void LoginHandler::getRegistrationDetails()
{
    // Not supported, so move on
    state = STATE_REGISTER;
}

void LoginHandler::loginAccount(LoginData *loginData)
{
    sendLoginRegister(loginData->username, loginData->password);
}

void LoginHandler::logout()
{
    // TODO
}

void LoginHandler::changeEmail(const std::string &email)
{
    // TODO
}

void LoginHandler::changePassword(const std::string &username,
                                  const std::string &oldPassword,
                                  const std::string &newPassword)
{
    MessageOut outMsg(CMSG_CHAR_PASSWORD_CHANGE);
    outMsg.writeString(oldPassword, 24);
    outMsg.writeString(newPassword, 24);
}

void LoginHandler::chooseServer(unsigned int server)
{
    if (server >= mWorlds.size())
        return;

    charServer.clear();
    charServer.hostname = ipToString(mWorlds[server]->address);
    charServer.port = mWorlds[server]->port;

    state = STATE_UPDATE;
}

void LoginHandler::registerAccount(LoginData *loginData)
{
    std::string username = loginData->username;
    username.append((loginData->gender == GENDER_FEMALE) ? "_F" : "_M");

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
     * at all. We're retasking it, as a bit mask:
     *  0 - can handle the 0x63 "update host" packet
     *  1 - defaults to the first char-server (instead of the last)
     */
    outMsg.writeInt8(0x03);
}

Worlds LoginHandler::getWorlds() const
{
    return mWorlds;
}

void LoginHandler::clearWorlds()
{
    delete_all(mWorlds);
    mWorlds.clear();
}

} // namespace EAthena
