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

#include "eventlistener.h"

#include "net/gamehandler.h"
#include "net/net.h"

#include "net/tmwa/messagehandler.h"
#include "net/tmwa/token.h"

namespace TmwAthena {

class GameHandler final : public MessageHandler, public Net::GameHandler,
        public EventListener
{
    public:
        GameHandler();

        void handleMessage(MessageIn &msg) override;

        void event(Event::Channel channel, const Event &event) override;

        void connect() override;

        bool isConnected() override;

        void disconnect() override;

        void quit() override;

        bool removeDeadBeings() const override { return true; }

        void clear();

        void setMap(const std::string &map);

        /** The tmwAthena protocol is making use of the MP status bar. */
        bool canUseMagicBar() const override { return true; }

        int getPickupRange() const override;

        int getNpcTalkRange() const override { return DEFAULT_TILE_LENGTH * 30; }

    private:
        std::string mMap; ///< Keeps the map filename.
        int mCharID; /// < Saved for map-server switching
        /**
         * Keeps the local character position until the map is loaded
         * to permit the translation in pixels.
         */
        int mTileX, mTileY;
};

} // namespace TmwAthena
