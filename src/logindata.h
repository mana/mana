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

#ifndef _TMW_LOGINDATA_H
#define _TMW_LOGINDATA_H

#include <string>

struct LoginData
{
    std::string username;
    std::string password;
    std::string hostname;
    std::string updateHost;
    short port;

    int account_ID;
    int session_ID1;
    int session_ID2;
    char sex;

    bool remember;            /**< Whether to store the username and host. */
    bool registerLogin;       /**< Whether an account is being registered. */
};

#endif
