/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef NET_MANASERV_NPCHANDLER_H
#define NET_MANASERV_NPCHANDLER_H

#include "net/npchandler.h"

#include "net/manaserv/messagehandler.h"

#include <map>

class NpcDialog;

namespace ManaServ {

class NpcHandler : public MessageHandler, public Net::NpcHandler
{
    public:
        NpcHandler();

        void handleMessage(Net::MessageIn &msg);

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

        void clearDialogs();

    private:
        typedef struct {
            NpcDialog* dialog;
        } Wrapper;
        typedef std::map<int, Wrapper> NpcDialogs;
        NpcDialogs mNpcDialogs;
};

} // namespace ManaServ

#endif // NET_MANASERV_NPCHANDLER_H
