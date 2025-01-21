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

#include "net/net.h"
#include "net/specialhandler.h"

#include "net/tmwa/messagehandler.h"

namespace TmwAthena {

class SpecialHandler final : public MessageHandler, public Net::SpecialHandler
{
    public:
        SpecialHandler();

        void handleMessage(MessageIn &msg) override;

        void use(int id) override;

        void use(int id, int level, int beingId) override;

        void use(int id, int level, int x, int y) override;

        void use(int id, const std::string &map) override;
};

} // namespace TmwAthena
