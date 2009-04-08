/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
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

#include "partytab.h"

#include "net/net.h"
#include "net/partyhandler.h"

#include "resources/iteminfo.h"
#include "resources/itemdb.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/strprintf.h"
#include "utils/stringutils.h"

PartyTab::PartyTab() :
    ChatTab(_("Party"))
{
}

PartyTab::~PartyTab()
{
}

void PartyTab::handleInput(const std::string &msg)
{
    Net::getPartyHandler()->chat(msg);
}

void PartyTab::handleCommand(std::string msg)
{
    std::string::size_type pos = msg.find(' ');
    std::string type(msg, 0, pos);
    std::string args(msg, pos == std::string::npos ? msg.size() : pos + 1);

    if (type == "help")
    {
        if (args == "")
        {
            chatLog(_("-- Help --"));
            chatLog(_("/help > Display this help."));
            chatLog(_("/create > Create a new party"));
            chatLog(_("/new > Alias of create"));
            chatLog(_("/invite > Invite a player to your party"));
            chatLog(_("/leave > Leave the party you are in"));
        }
        else if (args == "create" || args == "new")
        {
            chatLog(_("Command: /new <party-name>"));
            chatLog(_("Command: /create <party-name>"));
            chatLog(_("These commands create a new party called <party-name>."));
        }
        //else if (msg == "settings")
        //else if (msg == "info")
        else if (args == "invite")
        {
            chatLog(_("Command: /invite <nick>"));
            chatLog(_("This command invites <nick> to party with you."));
            chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
        }
        else if (args == "leave")
        {
            chatLog(_("Command: /leave"));
            chatLog(_("This command causes the player to leave the party."));
        }
        else if (args == "help")
        {
            chatLog(_("Command: /help"));
            chatLog(_("This command displays a list of all commands available."));
            chatLog(_("Command: /help <command>"));
            chatLog(_("This command displays help on <command>."));
        }
        else
        {
            chatLog(_("Unknown command."));
            chatLog(_("Type /help for a list of commands."));
        }
    }
    else if (type == "create" || type == "new")
    {
        if (args.empty())
            chatLog(_("Party name is missing."), BY_SERVER);
        else
            Net::getPartyHandler()->create(args);
    }
    else if (type == "invite")
    {
        Net::getPartyHandler()->invite(args);
    }
    else if (type == "leave")
    {
        Net::getPartyHandler()->leave();
    }
    else if (type == "settings")
    {
        chatLog(_("The settings command is not yet implemented!"));
        /*
        MessageOut outMsg(CMSG_PARTY_SETTINGS);
        outMsg.writeInt16(0); // Experience
        outMsg.writeInt16(0); // Item
        */
    }
    else
    {
        chatLog("Unknown command");
    }
}
