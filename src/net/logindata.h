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

#include "being.h"

#include <string>

class LoginData
{
public:
    LoginData() = default;

    std::string username;
    std::string password;
    std::string randomSeed;
    std::string newPassword;
    std::string updateHost;

    std::string email;
    std::string captchaResponse;

    Gender gender = Gender::Unspecified;

    bool remember;            /**< Whether to store the username. */
    bool registerLogin;       /**< Whether an account is being registered. */

    unsigned short characterSlots = 3;  /**< The number of character slots */

    /**
     * Initialize character slots to 3 for backwards compatibility
     */
    void resetCharacterSlots()
    {
        characterSlots = 3;
    }

    void clear()
    {
        username.clear();
        password.clear();
        randomSeed.clear();
        newPassword.clear();
        updateHost.clear();
        email.clear();
        captchaResponse.clear();
        gender = Gender::Unspecified;
        resetCharacterSlots();
    }
};
