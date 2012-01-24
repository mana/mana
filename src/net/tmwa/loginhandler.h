/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NET_TMWA_LOGINHANDLER_H
#define NET_TMWA_LOGINHANDLER_H

#include "net/loginhandler.h"

#include "net/tmwa/messagehandler.h"
#include "net/tmwa/token.h"

#include <string>

struct LoginData;

namespace TmwAthena {

class LoginHandler : public MessageHandler, public Net::LoginHandler
{
    public:
        LoginHandler();

        ~LoginHandler();

        void handleMessage(Net::MessageIn &msg);

        void connect();

        bool isConnected();

        void disconnect();

        int supportedOptionalActions() const
        { return SetGenderOnRegister; }

        bool isRegistrationEnabled();

        void getRegistrationDetails();

        unsigned int getMaxPasswordLength() const { return 25; }

        void loginAccount(LoginData *loginData);

        void logout();

        void changeEmail(const std::string &email);

        void changePassword(const std::string &username,
                            const std::string &oldPassword,
                            const std::string &newPassword);

        void chooseServer(unsigned int server);

        void registerAccount(LoginData *loginData);

        void unregisterAccount(const std::string &username,
                               const std::string &password);

        Worlds getWorlds() const;
        void clearWorlds();

        const Token &getToken() const { return mToken; }

    private:
        void sendLoginRegister(const std::string &username,
                               const std::string &password);

        bool mVersionResponse;
        bool mRegistrationEnabled;
        std::string mUpdateHost;
        Worlds mWorlds;
        Token mToken;
};

} // namespace TmwAthena

#endif // NET_TA_LOGINHANDLER_H
