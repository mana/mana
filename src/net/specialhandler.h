/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef SPECIALHANDLER_H
#define SPECIALHANDLER_H

#include <iosfwd>

namespace Net {
class SpecialHandler
{
    public:
        virtual ~SpecialHandler () {}

        virtual void use(int id) = 0;

        virtual void use(int id, int level, int beingId) = 0;

        virtual void use(int id, int level, int x, int y) = 0;

        virtual void use(int id, const std::string &map) = 0;
};
}

#endif // SPECIALHANDLER_H
