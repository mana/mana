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

#include "net/npchandler.h"

#include "net/manaserv/messagehandler.h"

namespace ManaServ {

class NpcHandler final : public MessageHandler, public Net::NpcHandler
{
    public:
        NpcHandler();

        void handleMessage(MessageIn &msg) override;

        void startShopping(int beingId) override;

        void buy(int beingId) override;

        void sell(int beingId) override;

        void buyItem(int beingId, int itemId, int amount) override;

        void sellItem(int beingId, int itemId, int amount) override;

        void endShopping(int beingId) override;

        void talk(int npcId) override;

        void nextDialog(int npcId) override;

        void closeDialog(int npcId) override;

        void menuSelect(int npcId, int choice) override;

        void integerInput(int npcId, int value) override;

        void stringInput(int npcId, const std::string &value) override;

        void sendLetter(int npcId, const std::string &recipient,
                        const std::string &text) override;

};

} // namespace ManaServ
