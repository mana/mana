/*
 *  The Mana Client
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#ifndef NET_MANASERV_SKILLHANDLER_H
#define NET_MANASERV_SKILLHANDLER_H

#include "net/specialhandler.h"

#include "net/manaserv/messagehandler.h"

namespace ManaServ {

class SpecialHandler : public MessageHandler, public Net::SpecialHandler
{
    public:
        SpecialHandler();

        void handleMessage(Net::MessageIn &msg);

        void use(int id);

        void use(int id, int level, int beingId);

        void use(int id, int level, int x, int y);

        void use(int id, const std::string &map);
};

} // namespace ManaServ

#endif // NET_MANASERV_SKILLHANDLER_H
