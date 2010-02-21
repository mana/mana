/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef NET_EA_GENERALHANDLER_H
#define NET_EA_GENERALHANDLER_H

#include "net/generalhandler.h"
#include "net/net.h"
#include "net/serverinfo.h"

#include "net/ea/messagehandler.h"

namespace EAthena {

class GeneralHandler : public MessageHandler, public Net::GeneralHandler
{
    public:
        GeneralHandler();

        ~GeneralHandler();

        void handleMessage(Net::MessageIn &msg);

        void load();

        void reload();

        void unload();

        void flushNetwork();

        void guiWindowsLoaded();

        void guiWindowsUnloaded();

        void clearHandlers();

    protected:
        MessageHandlerPtr mAdminHandler;
        MessageHandlerPtr mBeingHandler;
        MessageHandlerPtr mBuySellHandler;
        MessageHandlerPtr mCharHandler;
        MessageHandlerPtr mChatHandler;
        MessageHandlerPtr mGameHandler;
        MessageHandlerPtr mInventoryHandler;
        MessageHandlerPtr mItemHandler;
        MessageHandlerPtr mLoginHandler;
        MessageHandlerPtr mNpcHandler;
        MessageHandlerPtr mPartyHandler;
        MessageHandlerPtr mPlayerHandler;
        MessageHandlerPtr mSpecialHandler;
        MessageHandlerPtr mTradeHandler;
};

} // namespace EAthena

#endif // NET_EA_GENERALHANDLER_H
