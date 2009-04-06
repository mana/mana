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


#include "commandhandler.h"
#include "channelmanager.h"
#include "channel.h"
#include "game.h"
#include "localplayer.h"

#include "gui/widgets/channeltab.h"
#include "gui/widgets/chattab.h"
#include "gui/chat.h"

#include "net/adminhandler.h"
#include "net/chathandler.h"
#include "net/maphandler.h"
#include "net/net.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/strprintf.h"

CommandHandler::CommandHandler()
{}

void CommandHandler::handleCommand(const std::string &command, ChatTab *tab)
{
    std::string::size_type pos = command.find(' ');
    std::string type(command, 0, pos);
    std::string args(command, pos == std::string::npos ? command.size() : pos + 1);

    if (type == "announce")
    {
        handleAnnounce(args, tab);
    }
    else if (type == "help")
    {
        handleHelp(args, tab);
    }
    else if (type == "where")
    {
        handleWhere(args, tab);
    }
    else if (type == "who")
    {
        handleWho(args, tab);
    }
    else if (type == "msg" || type == "whisper" || type == "w")
    {
        handleMsg(args, tab);
    }
    else if (type == "join")
    {
        handleJoin(args, tab);
    }
    else if (type == "list")
    {
        handleListChannels(args, tab);
    }
    else if (type == "users")
    {
        handleListUsers(args, tab);
    }
    else if (type == "quit")
    {
        handleQuit(args, tab);
    }
    else if (type == "topic")
    {
        handleTopic(args, tab);
    }
    else if (type == "clear")
    {
        handleClear(args, tab);
    }
    else if (type == "op")
    {
        handleOp(args, tab);
    }
    else if (type == "kick")
    {
        handleKick(args, tab);
    }
    else if (type == "party")
    {
        handleParty(args, tab);
    }
    else if (type == "me")
    {
        handleMe(args, tab);
    }
    else if (type == "record")
    {
        handleRecord(args, tab);
    }
    else if (type == "toggle")
    {
        handleToggle(args, tab);
    }
    else if (type == "present")
    {
        handlePresent(args, tab);
    }
    else
    {
        tab->chatLog("Unknown command");
    }
}

void CommandHandler::handleAnnounce(const std::string &args, ChatTab *tab)
{
    Net::getAdminHandler()->announce(args);
}

void CommandHandler::handleHelp(const std::string &args, ChatTab *tab)
{
    if (args == "")
    {
        tab->chatLog(_("-- Help --"));
        tab->chatLog(_("/help > Display this help."));

        tab->chatLog(_("/where > Display map name"));
        tab->chatLog(_("/who > Display number of online users"));
        tab->chatLog(_("/me > Tell something about yourself"));

        tab->chatLog(_("/msg > Send a private message to a user"));
        tab->chatLog(_("/whisper > Alias of msg"));
        tab->chatLog(_("/w > Alias of msg"));
        tab->chatLog(_("/close > Close the whisper tab (only works in whisper tabs)"));

#ifdef TMWSERV_SUPPORT
        tab->chatLog(_("/list > Display all public channels"));
        tab->chatLog(_("/users > Lists the users in the current channel"));
        tab->chatLog(_("/join > Join or create a channel"));
        tab->chatLog(_("/topic > Set the topic of the current channel"));
        tab->chatLog(_("/quit > Leave a channel"));
        tab->chatLog(_("/clear > Clears this window"));
        tab->chatLog(_("/op > Make a user a channel operator"));
        tab->chatLog(_("/kick > Kick a user from the channel"));

        tab->chatLog(_("/party > Invite a user to party"));
#endif

        tab->chatLog(_("/record > Start recording the chat to an external file"));
        tab->chatLog(_("/toggle > Determine whether <return> toggles the chat log"));
        tab->chatLog(_("/present > Get list of players present (sent to chat log, if logging)"));

        tab->chatLog(_("/announce > Global announcement (GM only)"));

        tab->chatLog(_("For more information, type /help <command>"));
    }
    else if (args == "announce")
    {
        tab->chatLog(_("Command: /announce <msg>"));
        tab->chatLog(_("*** only available to a GM ***"));
        tab->chatLog(_("This command sends the message <msg> to "
                            "all players currently online."));
    }
    else if (args == "clear")
    {
        tab->chatLog(_("Command: /clear"));
        tab->chatLog(_("This command clears the chat log of previous chat."));
    }
    else if (args == "help")
    {
        tab->chatLog(_("Command: /help"));
        tab->chatLog(_("This command displays a list of all commands available."));
        tab->chatLog(_("Command: /help <command>"));
        tab->chatLog(_("This command displays help on <command>."));
    }
    else if (args == "join")
    {
        tab->chatLog(_("Command: /join <channel>"));
        tab->chatLog(_("This command makes you enter <channel>."));
        tab->chatLog(_("If <channel> doesn't exist, it's created."));
    }
    else if (args == "kick")
    {
        tab->chatLog(_("Command: /kick <nick>"));
        tab->chatLog(_("This command makes <nick> leave the channel."));
        tab->chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
    }
    else if (args == "list")
    {
        tab->chatLog(_("Command: /list"));
        tab->chatLog(_("This command shows a list of all channels."));
    }
    else if (args == "me")
    {
        tab->chatLog(_("Command: /me <message>"));
        tab->chatLog(_("This command tell others you are (doing) <msg>."));
    }
    else if (args == "msg" || args == "whisper" || args == "w")
    {
        tab->chatLog(_("Command: /msg <nick> <message>"));
        tab->chatLog(_("Command: /whisper <nick> <message>"));
        tab->chatLog(_("Command: /w <nick> <message>"));
        tab->chatLog(_("This command sends the text <message> to <nick>."));
        tab->chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
    }
    else if (args == "op")
    {
        tab->chatLog(_("Command: /op <nick>"));
        tab->chatLog(_("This command makes <nick> a channel operator."));
        tab->chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
        tab->chatLog(_("Channel operators can kick and op other users "
                            "from the channel."));
    }
    else if (args == "party")
    {
#ifdef TMWSERV_SUPPORT
        tab->chatLog(_("Command: /party <nick>"));
        tab->chatLog(_("This command invites <nick> to party with you."));
        tab->chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
#else
        tab->chatLog(_("Party commands are used on the party tab."));
#endif
    }
    else if (args == "present")
    {
        tab->chatLog(_("Command: /present"));
        tab->chatLog(_("This command gets a list of players within hearing and "
                  "sends it to either the record log if recording, or the chat "
                  "log otherwise."));
    }
    else if (args == "quit")
    {
        tab->chatLog(_("Command: /quit"));
        tab->chatLog(_("This command leaves the current channel."));
        tab->chatLog(_("If you're the last person in the channel, it will be deleted."));
    }
    else if (args == "record")
    {
        tab->chatLog(_("Command: /record <filename>"));
        tab->chatLog(_("This command starts recording the chat log to the file "
                  "<filename>."));
        tab->chatLog(_("Command: /record"));
        tab->chatLog(_("This command finishes a recording session."));
    }
    else if (args == "toggle")
    {
        tab->chatLog(_("Command: /toggle <state>"));
        tab->chatLog(_("This command sets whether the return key should toggle the "
                  "chat log, or whether the chat log turns off automatically."));
        tab->chatLog(_("<state> can be one of \"1\", \"yes\", \"true\" to "
                  "turn the toggle on, or \"0\", \"no\", \"false\" to turn the "
                  "toggle off."));
        tab->chatLog(_("Command: /toggle"));
        tab->chatLog(_("This command displays the return toggle status."));
    }
    else if (args == "topic")
    {
        tab->chatLog(_("Command: /topic <message>"));
        tab->chatLog(_("This command sets the topic to <message>."));
    }
    else if (args == "users")
    {
        tab->chatLog(_("Command: /users <channel>"));
        tab->chatLog(_("This command shows the users in <channel>."));
    }
    else if (args == "where")
    {
        tab->chatLog(_("Command: /where"));
        tab->chatLog(_("This command displays the name of the current map."));
    }
    else if (args == "who")
    {
        tab->chatLog(_("Command: /who"));
        tab->chatLog(_("This command displays the number of players currently "
                            "online."));
    }
    else
    {
        tab->chatLog(_("Unknown command."));
        tab->chatLog(_("Type /help for a list of commands."));
    }
}

void CommandHandler::handleWhere(const std::string &args, ChatTab *tab)
{
    // TODO: add position
    tab->chatLog(map_path, BY_SERVER);
}

void CommandHandler::handleWho(const std::string &args, ChatTab *tab)
{
    Net::getMapHandler()->who();
}

void CommandHandler::handleMsg(const std::string &args, ChatTab *tab)
{
    std::string recvnick = "";
    std::string msg = "";

    if (args.substr(0, 1) == "\"")
    {
        const std::string::size_type pos = args.find('"', 1);
        if (pos != std::string::npos)
        {
            recvnick = args.substr(1, pos - 1);
            if (pos + 2 < args.length())
                msg = args.substr(pos + 2, args.length());
        }
    }
    else
    {
        const std::string::size_type pos = args.find(" ");
        if (pos != std::string::npos)
        {
            recvnick = args.substr(0, pos);
            if (pos + 1 < args.length())
                msg = args.substr(pos + 1, args.length());
        }
        else
        {
            recvnick = std::string(args);
            msg = "";
        }
    }

    trim(msg);

    if (msg.length() > 0)
    {
        std::string playerName = player_node->getName();
        std::string tempNick = recvnick;

        toLower(playerName);
        toLower(tempNick);

        if (tempNick.compare(playerName) == 0 || args.empty())
            return;

        chatWindow->whisper(recvnick, msg, true);
    }
    else
        tab->chatLog("Cannont send empty whispers!");
}

void CommandHandler::handleClear(const std::string &args, ChatTab *tab)
{
    chatWindow->clearTab();
}

void CommandHandler::handleJoin(const std::string &args, ChatTab *tab)
{
    std::string::size_type pos = args.find(' ');
    std::string name(args, 0, pos);
    std::string password(args, pos+1);
    tab->chatLog("Requesting to join channel " + name);
    Net::getChatHandler()->enterChannel(name, password);
}

void CommandHandler::handleListChannels(const std::string &args, ChatTab *tab)
{
    Net::getChatHandler()->channelList();
}

void CommandHandler::handleListUsers(const std::string &args, ChatTab *tab)
{
    Net::getChatHandler()->userList(chatWindow->getFocused()->getCaption());
}

void CommandHandler::handleTopic(const std::string &args, ChatTab *tab)
{
    ChannelTab *channelTab = dynamic_cast<ChannelTab*>(tab);
    Channel *channel = channelTab ? channelTab->getChannel() : NULL;
    if (channel)
    {
        Net::getChatHandler()->setChannelTopic(channel->getId(), args);
    }
    else
    {
        tab->chatLog("Unable to set this channel's topic", BY_CHANNEL);
    }
}

void CommandHandler::handleQuit(const std::string &args, ChatTab *tab)
{
    ChannelTab *channelTab = dynamic_cast<ChannelTab*>(tab);
    Channel *channel = channelTab ? channelTab->getChannel() : NULL;
    if (channel)
    {
        Net::getChatHandler()->quitChannel(channel->getId());
    }
    else
    {
        tab->chatLog("Unable to quit this channel", BY_CHANNEL);
    }
}

void CommandHandler::handleOp(const std::string &args, ChatTab *tab)
{
    ChannelTab *channelTab = dynamic_cast<ChannelTab*>(tab);
    Channel *channel = channelTab ? channelTab->getChannel() : NULL;
    if (channel)
    {
        // set the user mode 'o' to op a user
        if (args != "")
        {
            Net::getChatHandler()->setUserMode(channel->getId(), args, 'o');
        }
    }
    else
    {
        tab->chatLog("Unable to set this user's mode", BY_CHANNEL);
    }
}

void CommandHandler::handleKick(const std::string &args, ChatTab *tab)
{
    ChannelTab *channelTab = dynamic_cast<ChannelTab*>(tab);
    Channel *channel = channelTab ? channelTab->getChannel() : NULL;
    if (channel)
    {
        if (args != "")
        {
            Net::getChatHandler()->kickUser(channel->getId(), args);
        }
    }
    else
    {
        tab->chatLog("Unable to kick user", BY_CHANNEL);
    }
}

void CommandHandler::handleParty(const std::string &args, ChatTab *tab)
{
#ifdef TMWSERV_SUPPORT
    if (args != "")
    {
        player_node->inviteToParty(args);
    }
#else
    tab->chatLog(_("Please use party commands on the party tab."));
#endif
}

void CommandHandler::handleMe(const std::string &args, ChatTab *tab)
{
    Net::getChatHandler()->me(args);
}

void CommandHandler::handleRecord(const std::string &args, ChatTab *tab)
{
    chatWindow->setRecordingFile(args);
}

void CommandHandler::handleToggle(const std::string &args, ChatTab *tab)
{
    if (args.empty())
    {
        tab->chatLog(chatWindow->getReturnTogglesChat() ?
                _("Return toggles chat.") : _("Message closes chat."));
        return;
    }

    std::string opt = args.substr(0, 1);

    if (opt == "1" ||
        opt == "y" || opt == "Y" ||
        opt == "t" || opt == "T")
    {
        tab->chatLog(_("Return now toggles chat."));
        chatWindow->setReturnTogglesChat(true);
        return;
    }
    else if (opt == "0" ||
             opt == "n" || opt == "N" ||
             opt == "f" || opt == "F")
    {
        tab->chatLog(_("Message now closes chat."));
        chatWindow->setReturnTogglesChat(false);
        return;
    }
    else
        tab->chatLog(_("Options to /toggle are \"yes\", \"no\", \"true\", "
                    "\"false\", \"1\", \"0\"."));
}

void CommandHandler::handlePresent(const std::string &args, ChatTab *tab)
{
    chatWindow->doPresent();
}
