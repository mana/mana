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

#ifndef NET_ACCOUNTSERVER_CHARACTER_H
#define NET_ACCOUNTSERVER_CHARACTER_H

#include <iosfwd>

namespace Net
{
    namespace AccountServer
    {
        namespace Account
        {
            void createCharacter(const std::string &name,
                    char hairStyle, char hairColor, char gender,
                    short strength, short agility, short vitality,
                    short intelligence, short dexterity, short willpower);

            void deleteCharacter(char slot);

            void selectCharacter(char slot);

            void unregister(const std::string &username,
                            const std::string &password);

            void changeEmail(const std::string &email);

            void changePassword(const std::string &username,
                                const std::string &oldPassword,
                                const std::string &newPassword);
        }
    }
}

#endif
