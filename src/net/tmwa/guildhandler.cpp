/*
 *  The Mana Client
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

#include "net/tmwa/guildhandler.h"

#include "guild.h"
#include "event.h"

#include "gui/socialwindow.h"

#include "net/tmwa/messagein.h"

#include "net/tmwa/gui/guildtab.h"

#include "utils/gettext.h"

extern Net::GuildHandler *guildHandler;

namespace TmwAthena {

GuildTab *guildTab = nullptr;
Guild *taGuild;

GuildHandler::GuildHandler()
{
    static const Uint16 _messages[] = {
        0
    };
    handledMessages = _messages;

    guildHandler = this;
}

GuildHandler::~GuildHandler()
{
    delete guildTab;
    guildTab = nullptr;
}

void GuildHandler::handleMessage(MessageIn &msg)
{
}

void GuildHandler::create(const std::string &name)
{
    serverNotice(_("Guild creation isn't supported."));
}

void GuildHandler::invite(int guildId, const std::string &name)
{
}

void GuildHandler::invite(int guildId, Being *being)
{
}

void GuildHandler::inviteResponse(int guildId, bool response)
{
}

void GuildHandler::leave(int guildId)
{
}

void GuildHandler::kick(GuildMember *member, std::string reason)
{
}

void GuildHandler::chat(int guildId, const std::string &text)
{
}

void GuildHandler::memberList(int guildId)
{
}

void GuildHandler::changeMemberPostion(GuildMember *member, int level)
{
}

void GuildHandler::requestAlliance(int guildId, int otherGuildId)
{
}

void GuildHandler::requestAllianceResponse(int guildId, int otherGuildId,
                             bool response)
{
}

void GuildHandler::endAlliance(int guildId, int otherGuildId)
{
}

} // namespace TmwAthena
