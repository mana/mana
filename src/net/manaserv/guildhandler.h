/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "net/guildhandler.h"

#include "net/manaserv/messagehandler.h"

namespace ManaServ {

class GuildHandler final : public Net::GuildHandler, public MessageHandler
{
public:
    GuildHandler();

    bool isSupported() override { return true; }

    void handleMessage(MessageIn &msg) override;

    void create(const std::string &name) override;

    void invite(int guildId, const std::string &name) override;

    void invite(int guidId, Being *being) override;

    void inviteResponse(int guidId, bool response) override;

    void leave(int guildId) override;

    void kick(GuildMember *member, std::string reason = std::string()) override;

    void chat(int guildId, const std::string &text) override;

    void memberList(int guildId) override;

    void changeMemberPostion(GuildMember *member, int level) override;

    void requestAlliance(int guildId, int otherGuildId) override;

    void requestAllianceResponse(int guildId, int otherGuildId,
                                 bool response) override;

    void endAlliance(int guildId, int otherGuildId) override;

protected:
    void joinedGuild(MessageIn &msg);
};

} // namespace ManaServ
