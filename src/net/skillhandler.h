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

#ifndef SKILLHANDLER_H
#define SKILLHANDLER_H

#include <iosfwd>

namespace Net {
class SkillHandler
{
    public:
        virtual void up(int skillId) = 0;

        virtual void use(int skillId, int level, int beingId) = 0;

        virtual void use(int skillId, int level, int x, int y) = 0;

        virtual void use(int skillId, const std::string &map) = 0;
};
}

#endif // SKILLHANDLER_H
