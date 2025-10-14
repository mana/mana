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

#pragma once

#include <iosfwd>

namespace Net {

class NpcHandler
{
    public:
        virtual ~NpcHandler() {}

        virtual void buy(int beingId) = 0;

        virtual void sell(int beingId) = 0;

        virtual void buyItem(int beingId, int itemId, int amount) = 0;

        virtual void sellItem(int beingId, int itemId, int amount) = 0;

        virtual void talk(int npcId) = 0;

        virtual void nextDialog(int npcId) = 0;

        virtual void closeDialog(int npcId) = 0;

        virtual void menuSelect(int npcId, int choice) = 0;

        virtual void integerInput(int npcId, int value) = 0;

        virtual void stringInput(int npcId, const std::string &value) = 0;

        virtual void sendLetter(int npcId, const std::string &recipient,
                                const std::string &text) = 0;

};

} // namespace Net
