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

#ifndef NET_MANASERV_LOGINHANDLER_H
#define NET_MANASERV_LOGINHANDLER_H

#include "net/loginhandler.h"
#include "net/serverinfo.h"

#include "net/manaserv/messagehandler.h"

class LoginData;

namespace ManaServ {

class LoginHandler : public MessageHandler, public Net::LoginHandler
{
    public:
        LoginHandler();

        void handleMessage(MessageIn &msg) override;

        void connect() override;

        bool isConnected() override;

        void disconnect() override;

        int supportedOptionalActions() const override
        { return Unregister | ChangeEmail | SetEmailOnRegister; }

        bool isRegistrationEnabled() override;

        void getRegistrationDetails() override;

        unsigned int getMinUserNameLength() const override;

        unsigned int getMaxUserNameLength() const override;

        void loginAccount(LoginData *loginData) override;

        void logout() override;

        void changeEmail(const std::string &email) override;

        void changePassword(const std::string &username,
                            const std::string &oldPassword,
                            const std::string &newPassword) override;

        void chooseServer(unsigned int server) override;

        void registerAccount(LoginData *loginData) override;

        void unregisterAccount(const std::string &username,
                               const std::string &password) override;

        Worlds getWorlds() const override;

        void reconnect();

    private:
        void handleLoginRandomResponse(MessageIn &msg);
        void handleLoginResponse(MessageIn &msg);
        void handleRegisterResponse(MessageIn &msg);

        void readServerInfo(MessageIn &msg);

        void loginAccountContinue();

        LoginData *mLoginData;
        std::string mTmpPassword;
        unsigned int mMinUserNameLength;
        unsigned int mMaxUserNameLength;
};

} // namespace ManaServ

#endif // NET_MANASERV_LOGINHANDLER_H
