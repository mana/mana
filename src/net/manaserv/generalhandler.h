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

#ifndef NET_MANASERV_GENERALHANDLER_H
#define NET_MANASERV_GENERALHANDLER_H

#include "net/generalhandler.h"
#include "net/net.h"

#include "net/manaserv/messagehandler.h"

namespace ManaServ {

class GeneralHandler : public Net::GeneralHandler
{
    public:
        GeneralHandler();

        void load();

        void reload();

        void unload();

        void flushNetwork();

        void guiWindowsLoaded();

        void guiWindowsUnloaded();

        void clearHandlers();

    protected:
        MessageHandlerPtr mBeingHandler;
        MessageHandlerPtr mBuySellHandler;
        MessageHandlerPtr mCharHandler;
        MessageHandlerPtr mChatHandler;
        MessageHandlerPtr mEffectHandler;
        MessageHandlerPtr mGameHandler;
        MessageHandlerPtr mGuildHandler;
        MessageHandlerPtr mInventoryHandler;
        MessageHandlerPtr mItemHandler;
        MessageHandlerPtr mLoginHandler;
        MessageHandlerPtr mNpcHandler;
        MessageHandlerPtr mPartyHandler;
        MessageHandlerPtr mPlayerHandler;
        MessageHandlerPtr mTradeHandler;
        MessageHandlerPtr mSpecialHandler;
};

} // namespace ManaServ

#endif // NET_MANASERV_GENERALHANDLER_H
