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

#ifndef NET_TMWSERV_NPCHANDLER_H
#define NET_TMWSERV_NPCHANDLER_H

#include "net/messagehandler.h"
#include "net/npchandler.h"

#include <list>

namespace TmwServ {

class NpcHandler : public MessageHandler, public Net::NpcHandler
{
    public:
        NpcHandler();

        void handleMessage(MessageIn &msg);

        void talk(int npcId);

        void nextDialog(int npcId);

        void closeDialog(int npcId);

        void listInput(int npcId, int value);

        void integerInput(int npcId, int value);

        void stringInput(int npcId, const std::string &value);

        void sendLetter(int npcId, const std::string &recipient,
                                const std::string &text);

        void startShopping(int beingId);

        void buy(int beingId);

        void sell(int beingId);

        void buyItem(int beingId, int itemId, int amount);

        void sellItem(int beingId, int itemId, int amount);

        void endShopping(int beingId);
};

} // namespace TmwServ

#endif
