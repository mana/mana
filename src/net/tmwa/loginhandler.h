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

#pragma once

#include "net/loginhandler.h"

#include "net/tmwa/messagehandler.h"
#include "net/tmwa/token.h"

#include <string>

class LoginData;

namespace TmwAthena {

class LoginHandler final : public MessageHandler, public Net::LoginHandler
{
    public:
        LoginHandler();

        ~LoginHandler() override;

        void handleMessage(MessageIn &msg) override;

        void connect() override;

        bool isConnected() override;

        void disconnect() override;

        int supportedOptionalActions() const override
        { return SetGenderOnRegister; }

        bool isRegistrationEnabled() override;

        void getRegistrationDetails() override;

        unsigned int getMaxPasswordLength() const override { return 25; }

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
        void clearWorlds();

        const Token &getToken() const { return mToken; }

        unsigned getServerVersion() const { return mServerVersion; }

    private:
        void sendLoginRegister(const std::string &username,
                               const std::string &password);

        unsigned mServerVersion = 0;
        bool mVersionResponse = false;
        bool mRegistrationEnabled = true;
        std::string mUpdateHost;
        Worlds mWorlds;
        Token mToken;
};

} // namespace TmwAthena
