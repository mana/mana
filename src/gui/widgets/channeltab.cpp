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

#include "channeltab.h"

#include "channel.h"

#include "net/chathandler.h"
#include "net/net.h"

#include "utils/gettext.h"

ChannelTab::ChannelTab(Channel *channel) :
    ChatTab(channel->getName()),
    mChannel(channel)
{
    channel->setTab(this);
}

ChannelTab::~ChannelTab()
{
}

void ChannelTab::handleInput(const std::string &msg)
{
    Net::getChatHandler()->sendToChannel(getChannel()->getId(), msg);
}

void ChannelTab::showHelp()
{
    chatLog(_("/users > Lists the users in the current channel"));
    chatLog(_("/topic > Set the topic of the current channel"));
    chatLog(_("/quit > Leave a channel"));
    chatLog(_("/op > Make a user a channel operator"));
    chatLog(_("/kick > Kick a user from the channel"));
}

bool ChannelTab::handleCommand(const std::string &type,
                               const std::string &args)
{
    if (type == "help")
    {
        if (args == "users")
        {
            chatLog(_("Command: /users"));
            chatLog(_("This command shows the users in this channel."));
        }
        else if (args == "topic")
        {
            chatLog(_("Command: /topic <message>"));
            chatLog(_("This command sets the topic to <message>."));
        }
        else if (args == "quit")
        {
            chatLog(_("Command: /quit"));
            chatLog(_("This command leaves the current channel."));
            chatLog(_("If you're the last person in the channel, it will be deleted."));
        }
        else if (args == "op")
        {
            chatLog(_("Command: /op <nick>"));
            chatLog(_("This command makes <nick> a channel operator."));
            chatLog(_("If the <nick> has spaces in it, enclose it in "
                                "double quotes (\")."));
            chatLog(_("Channel operators can kick and op other users "
                                "from the channel."));
        }
        else if (args == "kick")
        {
            chatLog(_("Command: /kick <nick>"));
            chatLog(_("This command makes <nick> leave the channel."));
            chatLog(_("If the <nick> has spaces in it, enclose it in "
                                "double quotes (\")."));
        }
        else
            return false;
    }
    else if (type == "users")
    {
        Net::getChatHandler()->userList(mChannel->getName());
    }
    else if (type == "topic")
    {
        Net::getChatHandler()->setChannelTopic(mChannel->getId(), args);
    }
    else if (type == "topic")
    {
        Net::getChatHandler()->setChannelTopic(mChannel->getId(), args);
    }
    else if (type == "quit")
    {
        Net::getChatHandler()->quitChannel(mChannel->getId());
    }
    else if (type == "op")
    {
        // set the user mode 'o' to op a user
        if (args != "")
            Net::getChatHandler()->setUserMode(mChannel->getId(), args, 'o');
        else
            chatLog(_("Need a user to op!"), BY_CHANNEL);
    }
    else if (type == "kick")
    {
        if (args != "")
            Net::getChatHandler()->kickUser(mChannel->getId(), args);
        else
            chatLog(_("Need a user to kick!"), BY_CHANNEL);
    }
    else
        return false;

    return true;
}
