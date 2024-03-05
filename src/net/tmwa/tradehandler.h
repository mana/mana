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

#ifndef NET_TA_TRADEHANDLER_H
#define NET_TA_TRADEHANDLER_H

#include "net/net.h"
#include "net/tradehandler.h"

#include "net/tmwa/messagehandler.h"

namespace TmwAthena {

class TradeHandler final : public MessageHandler, public Net::TradeHandler
{
    public:
        TradeHandler();

        void handleMessage(MessageIn &msg) override;

        void request(Being *being) override;

        void respond(bool accept) override;

        void addItem(Item *item, int amount) override;

        void removeItem(int slotNum, int amount) override;

        void setMoney(int amount) override;

        void confirm() override;

        void finish() override;

        void cancel() override;

    private:
        bool mTrading;
};

} // namespace TmwAthena

#endif // NET_TA_TRADEHANDLER_H
