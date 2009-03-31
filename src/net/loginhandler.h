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

#include <iosfwd>

namespace Net {
    class LoginHandler
    {
        public:
            virtual void loginAccount(const std::string &username,
                               const std::string &password);
            virtual void changeEmail(const std::string &email) {}
            virtual void changePassword(const std::string &username,
                               const std::string &oldPassword,
                               const std::string &newPassword) {}
            virtual void chooseServer(int server) {}
            virtual void registerAccount(const std::string &username,
                               const std::string &password,
                               const std::string &email = "");
            virtual void unregisterAccount(const std::string &username,
                               const std::string &password);
    };
}

#endif // LOGINHANDLER_H
