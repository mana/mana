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

#include "commandhandler.h"

#include "gui/palette.h"

#include "net/net.h"
#include "net/partyhandler.h"

#include "resources/iteminfo.h"
#include "resources/itemdb.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

PartyTab::PartyTab() :
    ChatTab(_("Party"))
{
    setTabColor(&guiPalette->getColor(Palette::PARTY));
}

PartyTab::~PartyTab()
{
}

void PartyTab::handleInput(const std::string &msg)
{
    Net::getPartyHandler()->chat(msg);
}

void PartyTab::showHelp()
{
    chatLog(_("/help > Display this help."));
    chatLog(_("/create > Create a new party"));
    chatLog(_("/new > Alias of create"));
    chatLog(_("/invite > Invite a player to your party"));
    chatLog(_("/leave > Leave the party you are in"));
    chatLog(_("/kick > Kick some one from the party you are in"));
    chatLog(_("/item > Show/change party item sharing options"));
    chatLog(_("/exp > Show/change party experience sharing options"));
}

bool PartyTab::handleCommand(const std::string &type, const std::string &args)
{
    if (type == "help")
    {
        if (args == "create" || args == "new")
        {
            chatLog(_("Command: /new <party-name>"));
            chatLog(_("Command: /create <party-name>"));
            chatLog(_("These commands create a new party called <party-name>."));
        }
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
        else if (args == "item")
        {
            chatLog(_("Command: /item <policy>"));
            chatLog(_("This command changes the party's item sharing policy."));
            chatLog(_("<policy> can be one of \"1\", \"yes\", \"true\" to "
                      "enable item sharing, or \"0\", \"no\", \"false\" to "
                      "disable item sharing."));
            chatLog(_("Command: /item"));
            chatLog(_("This command displays the party's current item sharing policy."));
        }
        else if (args == "exp")
        {
            chatLog(_("Command: /exp <policy>"));
            chatLog(_("This command changes the party's experience sharing policy."));
            chatLog(_("<policy> can be one of \"1\", \"yes\", \"true\" to "
                      "enable experience sharing, or \"0\", \"no\", \"false\" to "
                      "disable experience sharing."));
            chatLog(_("Command: /exp"));
            chatLog(_("This command displays the party's current experience sharing policy."));
        }
        else
            return false;
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
    else if (type == "kick")
    {
        Net::getPartyHandler()->kick(args);
    }
    else if (type == "item")
    {
        if (args.empty())
        {
            switch (Net::getPartyHandler()->getShareItems())
            {
                case PARTY_SHARE:
                    chatLog(_("Item sharing enabled."), BY_SERVER);
                    return true;
                case PARTY_SHARE_NO:
                    chatLog(_("Item sharing disabled."), BY_SERVER);
                    return true;
                case PARTY_SHARE_NOT_POSSIBLE:
                    chatLog(_("Item sharing not possible."), BY_SERVER);
                    return true;
            }
        }

        char opt = CommandHandler::parseBoolean(args);

        switch (opt)
        {
            case 1:
                Net::getPartyHandler()->setShareItems(PARTY_SHARE);
                break;
            case 0:
                Net::getPartyHandler()->setShareItems(PARTY_SHARE_NO);
                break;
            case -1:
                chatLog(strprintf(BOOLEAN_OPTIONS, "item"));
        }
    }
    else if (type == "exp")
    {
        if (args.empty())
        {
            switch (Net::getPartyHandler()->getShareExperience())
            {
                case PARTY_SHARE:
                    chatLog(_("Experience sharing enabled."), BY_SERVER);
                    return true;
                case PARTY_SHARE_NO:
                    chatLog(_("Experience sharing disabled."), BY_SERVER);
                    return true;
                case PARTY_SHARE_NOT_POSSIBLE:
                    chatLog(_("Experience sharing not possible."), BY_SERVER);
                    return true;
            }
        }

        char opt = CommandHandler::parseBoolean(args);

        switch (opt)
        {
            case 1:
                Net::getPartyHandler()->setShareExperience(PARTY_SHARE);
                break;
            case 0:
                Net::getPartyHandler()->setShareExperience(PARTY_SHARE_NO);
                break;
            case -1:
                chatLog(strprintf(BOOLEAN_OPTIONS, "exp"));
        }
    }
    else
        return false;

    return true;
}
