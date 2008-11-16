/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _TMW_NET_TRADEHANDLER_H
#define _TMW_NET_TRADEHANDLER_H

#include "messagehandler.h"

class TradeHandler : public MessageHandler
{
    public:
        TradeHandler();

        void handleMessage(MessageIn &msg);

        /**
         * Returns whether trade requests are accepted.
         *
         * @see setAcceptTradeRequests
         */
        bool acceptTradeRequests() const
        { return mAcceptTradeRequests; }

        /**
         * Sets whether trade requests are accepted. When set to false, trade
         * requests are automatically denied. When true, a popup will ask the
         * player whether he wants to trade.
         */
        void setAcceptTradeRequests(bool acceptTradeRequests);

    private:
        bool mAcceptTradeRequests;
};

#endif
