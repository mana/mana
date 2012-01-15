/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "net/tmwa/gui/guildtab.h"

#include "chatlogger.h"
#include "commandhandler.h"
#include "guild.h"
#include "localplayer.h"

#include "net/net.h"
#include "net/guildhandler.h"

#include "resources/iteminfo.h"
#include "resources/itemdb.h"
#include "resources/theme.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

namespace TmwAthena {

extern Guild *taGuild;

GuildTab::GuildTab() :
    ChatTab(_("Guild"))
{
    setTabColor(&Theme::getThemeColor(Theme::GUILD));
}

GuildTab::~GuildTab()
{
}

void GuildTab::handleInput(const std::string &msg)
{
    Net::getGuildHandler()->chat(taGuild->getId(), msg);
}

void GuildTab::showHelp()
{
    chatLog(_("/help > Display this help."));
    chatLog(_("/invite > Invite a player to your guild"));
    chatLog(_("/leave > Leave the guild you are in"));
    chatLog(_("/kick > Kick some one from the guild you are in"));
}

bool GuildTab::handleCommand(const std::string &type, const std::string &args)
{
    if (type == "help")
    {
        if (args == "invite")
        {
            chatLog(_("Command: /invite <nick>"));
            chatLog(_("This command invites <nick> to the guild you're in."));
            chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
        }
        else if (args == "leave")
        {
            chatLog(_("Command: /leave"));
            chatLog(_("This command causes the player to leave the guild."));
        }
        else
            return false;
    }
    else if (type == "create" || type == "new")
    {
        if (args.empty())
            chatLog(_("Guild name is missing."), BY_SERVER);
        else
            Net::getGuildHandler()->create(args);
    }
    else if (type == "invite")
    {
        Net::getGuildHandler()->invite(taGuild->getId(), args);
    }
    else if (type == "leave")
    {
        Net::getGuildHandler()->leave(taGuild->getId());
    }
    else if (type == "kick")
    {
        Net::getGuildHandler()->kick(taGuild->getMember(args));
    }
    else
        return false;

    return true;
}

void GuildTab::getAutoCompleteList(std::vector<std::string> &names) const
{
    if (taGuild)
        taGuild->getNames(names);
}

void GuildTab::saveToLogFile(std::string &msg)
{
    if (chatLogger)
        chatLogger->log("#Guild", msg);
}

} // namespace TmwAthena
