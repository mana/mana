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

#ifndef MAPHANDLER_H
#define MAPHANDLER_H

#include "logindata.h"

#include <iosfwd>

namespace Net {

class GameHandler
{
    public:
        virtual void connect() = 0;

        virtual bool isConnected() = 0;

        virtual void disconnect() = 0;

        virtual void inGame() = 0;

        virtual void mapLoaded(const std::string &mapName) = 0;

        virtual void who() = 0;

        virtual void quit() = 0;

        virtual void ping(int tick) = 0;

        virtual bool removeDeadBeings() const = 0;

        virtual ~GameHandler() {}
};

} // namespace Net

#endif // MAPHANDLER_H
