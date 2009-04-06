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

#ifndef NPCHANDLER_H
#define NPCHANDLER_H

#include <iosfwd>

namespace Net {

class NpcHandler
{
    public:
        virtual void talk(int npcId) = 0;

        virtual void nextDialog(int npcId) = 0;

        virtual void closeDialog(int npcId) = 0;

        virtual void listInput(int npcId, int value) = 0;

        virtual void integerInput(int npcId, int value) = 0;

        virtual void stringInput(int npcId, const std::string &value) = 0;

        virtual void sendLetter(int npcId, const std::string &recipient,
                                const std::string &text) = 0;

        virtual void startShopping(int beingId) = 0;

        virtual void buy(int beingId) = 0;

        virtual void sell(int beingId) = 0;

        virtual void buyItem(int beingId, int itemId, int amount) = 0;

        virtual void sellItem(int beingId, int itemId, int amount) = 0;

        virtual void endShopping(int beingId) = 0;
};

} // namespace Net

#endif // NPCHANDLER_H
