/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef LOGINHANDLER_H
#define LOGINHANDLER_H

#include "logindata.h"

#include "net/serverinfo.h"
#include "net/worldinfo.h"

#include <iosfwd>
#include <vector>

namespace Net {

class LoginHandler
{
    public:
        enum OptionalAction {
            Unregister  = 0x1,
            ChangeEmail = 0x2
        };

        virtual void setServer(const ServerInfo &server)
        { mServer = server; }

        virtual ServerInfo getServer()
        { return mServer; }

        virtual void connect() = 0;

        virtual bool isConnected() = 0;

        virtual void disconnect() = 0;

        virtual int supportedOptionalActions() const = 0;

        virtual void loginAccount(LoginData *loginData) = 0;

        virtual void logout() = 0;

        virtual void changeEmail(const std::string &email) = 0;

        virtual void changePassword(const std::string &username,
                                    const std::string &oldPassword,
                                    const std::string &newPassword) = 0;

        virtual void chooseServer(unsigned int server) = 0;

        virtual void registerAccount(LoginData *loginData) = 0;

        virtual void unregisterAccount(const std::string &username,
                                       const std::string &password) = 0;

        virtual Worlds getWorlds() = 0;

    protected:
        ServerInfo mServer;
};

} // namespace Net

#endif // LOGINHANDLER_H
