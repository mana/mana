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

#include "net/ea/logouthandler.h"

#include "net/ea/protocol.h"

#include "net/logindata.h"
#include "net/messagein.h"
#include "net/messageout.h"
#include "net/serverinfo.h"

#include "log.h"
#include "main.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

Net::LogoutHandler *logoutHandler;

namespace EAthena {

LogoutHandler::LogoutHandler()
{
    static const Uint16 _messages[] = {
        0
    };
    handledMessages = _messages;
    logoutHandler = this;
}

void LogoutHandler::handleMessage(MessageIn &msg)
{
}

void LogoutHandler::setScenario(unsigned short scenario,
                                std::string *passToken)
{
    // TODO
}

void LogoutHandler::reset()
{
    // TODO
}

} // namespace EAthena
