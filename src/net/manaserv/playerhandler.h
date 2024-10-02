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

#ifndef NET_MANASERV_PLAYERHANDLER_H
#define NET_MANASERV_PLAYERHANDLER_H

#include "net/playerhandler.h"

#include "net/manaserv/messagehandler.h"

#include <guichan/actionlistener.hpp>

namespace ManaServ {

struct RespawnRequestListener final : public gcn::ActionListener
{
    void action(const gcn::ActionEvent &event) override;
};
static RespawnRequestListener respawnListener;

class PlayerHandler final : public MessageHandler, public Net::PlayerHandler
{
    public:
        PlayerHandler();

        void handleMessage(MessageIn &msg) override;

        void attack(int id) override;
        void emote(int emoteId) override;

        void increaseAttribute(int attr) override;
        void decreaseAttribute(int attr) override;
        void increaseSkill(int skillId) override;

        void pickUp(FloorItem *floorItem) override;
        void setDirection(char direction) override;
        void setDestination(int x, int y, int direction = -1) override;
        void changeAction(Being::Action action) override;

        void respawn() override;

        void ignorePlayer(const std::string &player, bool ignore) override;
        void ignoreAll(bool ignore) override;

        bool canUseMagic() override;
        bool canCorrectAttributes() override;

        int getJobLocation() override;

        Vector getDefaultMoveSpeed() const override;

        Vector getPixelsPerSecondMoveSpeed(const Vector &speed, Map *map = nullptr) override;

        bool usePixelPrecision() override
        { return true; }

    private:
        void handleMapChangeMessage(MessageIn &msg);
};

} // namespace ManaServ

#endif // NET_MANASERV_PLAYERHANDLER_H
