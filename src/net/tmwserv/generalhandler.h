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

#ifndef NET_TMWSERV_GENERALHANDLER_H
#define NET_TMWSERV_GENERALHANDLER_H

#include "net/generalhandler.h"
#include "net/net.h"
#include "net/messagehandler.h"

namespace TmwServ {

class GeneralHandler : public Net::GeneralHandler
{
    public:
        GeneralHandler();

        void load();

        void unload();

        void flushNetwork();

        bool isNetworkConnected();

        void tick();

        void guiWindowsLoaded();

        void guiWindowsUnloaded();

    protected:
        MessageHandlerPtr mBeingHandler;
        MessageHandlerPtr mBuySellHandler;
        MessageHandlerPtr mCharServerHandler;
        MessageHandlerPtr mChatHandler;
        MessageHandlerPtr mEffectHandler;
        MessageHandlerPtr mGuildHandler;
        MessageHandlerPtr mInventoryHandler;
        MessageHandlerPtr mItemHandler;
        MessageHandlerPtr mLoginHandler;
        MessageHandlerPtr mLogoutHandler;
        MessageHandlerPtr mMapHandler;
        MessageHandlerPtr mNpcHandler;
        MessageHandlerPtr mPartyHandler;
        MessageHandlerPtr mPlayerHandler;
        MessageHandlerPtr mTradeHandler;
};

} // namespace TmwServ

#endif // NET_TMWSERV_GENERALHANDLER_H
