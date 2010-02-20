/*
 *  The Mana Client
 *  Copyright (C) 2009-2010  The Mana World Development Team
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

#ifndef ADMINHANDLER_H
#define ADMINHANDLER_H

#include <iosfwd>

namespace Net {

class AdminHandler
{
    public:
        virtual void announce(const std::string &text) = 0;

        virtual void localAnnounce(const std::string &text) = 0;

        virtual void hide(bool hide) = 0;

        virtual void kick(int playerId) = 0;

        virtual void kick(const std::string &name) = 0;

        virtual void ban(int playerId) = 0;

        virtual void ban(const std::string &name) = 0;

        virtual void unban(int playerId) = 0;

        virtual void unban(const std::string &name) = 0;

        virtual void mute(int playerId, int type, int limit) = 0;

        virtual ~AdminHandler() {}

        // TODO
};

} // namespace Net

#endif // ADMINHANDLER_H
