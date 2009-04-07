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

#ifndef LOGOUTHANDLER_H
#define LOGOUTHANDLER_H

#include "logindata.h"

#include <iosfwd>

/**
 * The different scenarios for which LogoutHandler can be used
 */
enum {
    LOGOUT_EXIT,
    LOGOUT_SWITCH_LOGIN,
    LOGOUT_SWITCH_CHARACTER
};

namespace Net {
class LogoutHandler
{
    public:
        virtual void setScenario(unsigned short scenario,
                         std::string *passToken = NULL) = 0;

        virtual void reset() = 0;
};
}

#endif // LOGOUTHANDLER_H
