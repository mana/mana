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

#ifdef TMWSERV_SUPPORT
#include "net/tmwserv/chatserver/chatserver.h"
#include "net/tmwserv/gameserver/player.h"
#else
#include "party.h"
#include "net/messageout.h"
#include "net/ea/protocol.h"
#endif

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/strprintf.h"

#ifdef TMWSERV_SUPPORT
CommandHandler::CommandHandler()
#else
CommandHandler::CommandHandler(Network *network):
    mNetwork(network)
#endif
{}

void CommandHandler::handleCommand(const std::string &command)
{
    std::string::size_type pos = command.find(' ');
    std::string type(command, 0, pos);
    std::string args(command, pos == std::string::npos ? command.size() : pos + 1);

    if (type == "announce")
    {
        handleAnnounce(args);
    }
    else if (type == "help")
    {
        handleHelp(args);
    }
    else if (type == "where")
    {
        handleWhere();
    }
    else if (type == "who")
    {
        handleWho();
    }
    else if (type == "msg" || type == "whisper" || type == "w")
    {
        handleMsg(args);
    }
#ifdef TMWSERV_SUPPORT
    else if (type == "join")
    {
        handleJoin(args);
    }
    else if (type == "list")
    {
        handleListChannels();
    }
    else if (type == "users")
    {
        handleListUsers();
    }
    else if (type == "quit")
    {
        handleQuit();
    }
    else if (type == "topic")
    {
        handleTopic(args);
    }
    else if (type == "clear")
    {
        handleClear();
    }
    else if (type == "op")
    {
        handleOp(args);
    }
    else if (type == "kick")
    {
        handleKick(args);
    }
#endif
    else if (type == "party")
    {
        handleParty(args);
    }
    else if (type == "me")
    {
        handleMe(args);
    }
    else if (type == "record")
    {
        handleRecord(args);
    }
    else if (type == "toggle")
    {
        handleToggle(args);
    }
    else if (type == "present")
    {
        handlePresent(args);
    }
    else
    {
        localChatTab->chatLog("Unknown command");
    }
}

void CommandHandler::handleAnnounce(const std::string &args)
{
#ifdef TMWSERV_SUPPORT
    Net::ChatServer::announce(args);
#else
    MessageOut outMsg(0x0099);
    outMsg.writeInt16(args.length() + 4);
    outMsg.writeString(args, args.length());
#endif
}

void CommandHandler::handleHelp(const std::string &args)
{
    if (args == "")
    {
        localChatTab->chatLog(_("-- Help --"));
        localChatTab->chatLog(_("/help > Display this help."));

        localChatTab->chatLog(_("/where > Display map name"));
        localChatTab->chatLog(_("/who > Display number of online users"));
        localChatTab->chatLog(_("/me > Tell something about yourself"));

        localChatTab->chatLog(_("/msg > Send a private message to a user"));
        localChatTab->chatLog(_("/whisper > Alias of msg"));
        localChatTab->chatLog(_("/w > Alias of msg"));
        localChatTab->chatLog(_("/close > Close the whisper tab (only works in whisper tabs)"));

#ifdef TMWSERV_SUPPORT
        localChatTab->chatLog(_("/list > Display all public channels"));
        localChatTab->chatLog(_("/users > Lists the users in the current channel"));
        localChatTab->chatLog(_("/join > Join or create a channel"));
        localChatTab->chatLog(_("/topic > Set the topic of the current channel"));
        localChatTab->chatLog(_("/quit > Leave a channel"));
        localChatTab->chatLog(_("/clear > Clears this window"));
        localChatTab->chatLog(_("/op > Make a user a channel operator"));
        localChatTab->chatLog(_("/kick > Kick a user from the channel"));

        localChatTab->chatLog(_("/party > Invite a user to party"));
#else
        localChatTab->chatLog(_("/party > Party-related commands"));
#endif

        localChatTab->chatLog(_("/record > Start recording the chat to an external file"));
        localChatTab->chatLog(_("/toggle > Determine whether <return> toggles the chat log"));
        localChatTab->chatLog(_("/present > Get list of players present (sent to chat log, if logging)"));

        localChatTab->chatLog(_("/announce > Global announcement (GM only)"));

        localChatTab->chatLog(_("For more information, type /help <command>"));
    }
    else if (args == "announce")
    {
        localChatTab->chatLog(_("Command: /announce <msg>"));
        localChatTab->chatLog(_("*** only available to a GM ***"));
        localChatTab->chatLog(_("This command sends the message <msg> to "
                            "all players currently online."));
    }
    else if (args == "clear")
    {
        localChatTab->chatLog(_("Command: /clear"));
        localChatTab->chatLog(_("This command clears the chat log of previous chat."));
    }
    else if (args == "help")
    {
        localChatTab->chatLog(_("Command: /help"));
        localChatTab->chatLog(_("This command displays a list of all commands available."));
        localChatTab->chatLog(_("Command: /help <command>"));
        localChatTab->chatLog(_("This command displays help on <command>."));
    }
    else if (args == "join")
    {
        localChatTab->chatLog(_("Command: /join <channel>"));
        localChatTab->chatLog(_("This command makes you enter <channel>."));
        localChatTab->chatLog(_("If <channel> doesn't exist, it's created."));
    }
    else if (args == "kick")
    {
        localChatTab->chatLog(_("Command: /kick <nick>"));
        localChatTab->chatLog(_("This command makes <nick> leave the channel."));
        localChatTab->chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
    }
    else if (args == "list")
    {
        localChatTab->chatLog(_("Command: /list"));
        localChatTab->chatLog(_("This command shows a list of all channels."));
    }
    else if (args == "me")
    {
        localChatTab->chatLog(_("Command: /me <message>"));
        localChatTab->chatLog(_("This command tell others you are (doing) <msg>."));
    }
    else if (args == "msg" || args == "whisper" || args == "w")
    {
        localChatTab->chatLog(_("Command: /msg <nick> <message>"));
        localChatTab->chatLog(_("Command: /whisper <nick> <message>"));
        localChatTab->chatLog(_("Command: /w <nick> <message>"));
        localChatTab->chatLog(_("This command sends the text <message> to <nick>."));
        localChatTab->chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
    }
    else if (args == "op")
    {
        localChatTab->chatLog(_("Command: /op <nick>"));
        localChatTab->chatLog(_("This command makes <nick> a channel operator."));
        localChatTab->chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
        localChatTab->chatLog(_("Channel operators can kick and op other users "
                            "from the channel."));
    }
#ifdef TMWSERV_SUPPORT
    else if (args == "party")
    {
        localChatTab->chatLog(_("Command: /party <nick>"));
        localChatTab->chatLog(_("This command invites <nick> to party with you."));
        localChatTab->chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
#else
    else if (args.substr(0, 5) == "party")
    {
        playerParty->help(args);
#endif
    }
    else if (args == "present")
    {
        localChatTab->chatLog(_("Command: /present"));
        localChatTab->chatLog(_("This command gets a list of players within hearing and "
                  "sends it to either the record log if recording, or the chat "
                  "log otherwise."));
    }
    else if (args == "quit")
    {
        localChatTab->chatLog(_("Command: /quit"));
        localChatTab->chatLog(_("This command leaves the current channel."));
        localChatTab->chatLog(_("If you're the last person in the channel, it will be deleted."));
    }
    else if (args == "record")
    {
        localChatTab->chatLog(_("Command: /record <filename>"));
        localChatTab->chatLog(_("This command starts recording the chat log to the file "
                  "<filename>."));
        localChatTab->chatLog(_("Command: /record"));
        localChatTab->chatLog(_("This command finishes a recording session."));
    }
    else if (args == "toggle")
    {
        localChatTab->chatLog(_("Command: /toggle <state>"));
        localChatTab->chatLog(_("This command sets whether the return key should toggle the "
                  "chat log, or whether the chat log turns off automatically."));
        localChatTab->chatLog(_("<state> can be one of \"1\", \"yes\", \"true\" to "
                  "turn the toggle on, or \"0\", \"no\", \"false\" to turn the "
                  "toggle off."));
        localChatTab->chatLog(_("Command: /toggle"));
        localChatTab->chatLog(_("This command displays the return toggle status."));
    }
    else if (args == "topic")
    {
        localChatTab->chatLog(_("Command: /topic <message>"));
        localChatTab->chatLog(_("This command sets the topic to <message>."));
    }
    else if (args == "users")
    {
        localChatTab->chatLog(_("Command: /users <channel>"));
        localChatTab->chatLog(_("This command shows the users in <channel>."));
    }
    else if (args == "where")
    {
        localChatTab->chatLog(_("Command: /where"));
        localChatTab->chatLog(_("This command displays the name of the current map."));
    }
    else if (args == "who")
    {
        localChatTab->chatLog(_("Command: /who"));
        localChatTab->chatLog(_("This command displays the number of players currently "
                            "online."));
    }
    else
    {
        localChatTab->chatLog(_("Unknown command."));
        localChatTab->chatLog(_("Type /help for a list of commands."));
    }
}

void CommandHandler::handleWhere()
{
    // TODO: add position
    localChatTab->chatLog(map_path, BY_SERVER);
}

void CommandHandler::handleWho()
{
#ifdef TMWSERV_SUPPORT
    //TODO
#else
    MessageOut outMsg(0x00c1);
#endif
}

void CommandHandler::handleMsg(const std::string &args)
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
        localChatTab->chatLog("Cannont send empty whispers!");
}

void CommandHandler::handleClear()
{
    chatWindow->clearTab();
}

#ifdef TMWSERV_SUPPORT

void CommandHandler::handleJoin(const std::string &args)
{
    std::string::size_type pos = args.find(' ');
    std::string name(args, 0, pos);
    std::string password(args, pos+1);
    localChatTab->chatLog("Requesting to join channel " + name);
    Net::ChatServer::enterChannel(name, password);
}

void CommandHandler::handleListChannels()
{
    Net::ChatServer::getChannelList();
}

void CommandHandler::handleListUsers()
{
    Net::ChatServer::getUserList(chatWindow->getFocused()->getCaption());
}

void CommandHandler::handleTopic(const std::string &args)
{
    ChannelTab *tab = dynamic_cast<ChannelTab*>(chatWindow->getFocused());
    Channel *channel = tab ? tab->getChannel() : NULL;
    if (channel)
    {
        Net::ChatServer::setChannelTopic(channel->getId(), args);
    }
    else
    {
        localChatTab->chatLog("Unable to set this channel's topic", BY_CHANNEL);
    }
}

void CommandHandler::handleQuit()
{
    ChannelTab *tab = dynamic_cast<ChannelTab*>(chatWindow->getFocused());
    Channel *channel = tab ? tab->getChannel() : NULL;
    if (channel)
    {
        Net::ChatServer::quitChannel(channel->getId());
    }
    else
    {
        localChatTab->chatLog("Unable to quit this channel", BY_CHANNEL);
    }
}

void CommandHandler::handleOp(const std::string &args)
{
    ChannelTab *tab = dynamic_cast<ChannelTab*>(chatWindow->getFocused());
    Channel *channel = tab ? tab->getChannel() : NULL;
    if (channel)
    {
        // set the user mode 'o' to op a user
        if (args != "")
        {
            Net::ChatServer::setUserMode(channel->getId(), args, 'o');
        }
    }
    else
    {
        localChatTab->chatLog("Unable to set this user's mode", BY_CHANNEL);
    }
}

void CommandHandler::handleKick(const std::string &args)
{
    ChannelTab *tab = dynamic_cast<ChannelTab*>(chatWindow->getFocused());
    Channel *channel = tab ? tab->getChannel() : NULL;
    if (channel)
    {
        if (args != "")
        {
            Net::ChatServer::kickUser(channel->getId(), args);
        }
    }
    else
    {
        localChatTab->chatLog("Unable to kick user", BY_CHANNEL);
    }
}

#endif

void CommandHandler::handleParty(const std::string &args)
{
#ifdef TMWSERV_SUPPORT
    if (args != "")
    {
        player_node->inviteToParty(args);
    }
#else
    if (args.empty())
    {
        localChatTab->chatLog(_("Unknown party command... Type \"/help\" party for more "
                "information."), BY_SERVER);
        return;
    }

    const std::string::size_type space = args.find(" ");
    std::string command;
    std::string rest;

    if (space == std::string::npos)
    {
        command = args;
    }
    else
    {
        command = args.substr(0, space);
        rest = args.substr(space + 1, args.length());
    }

    if (command == "prefix")
    {
        if (rest.empty())
        {
            char temp[2] = ".";
            *temp = chatWindow->getPartyPrefix();
            localChatTab->chatLog(_("The current party prefix is ") + std::string(temp));
        }
        else if (rest.length() != 1)
        {
            localChatTab->chatLog(_("Party prefix must be one character long."));
        }
        else
        {
            if (rest == "/")
            {
                localChatTab->chatLog(_("Cannot use a '/' as the prefix."));
            }
            else
            {
                chatWindow->setPartyPrefix(rest.at(0));
                localChatTab->chatLog(_("Changing prefix to ") + rest);
            }
        }
    }
    else
        playerParty->respond(command, rest);
#endif
}

void CommandHandler::handleMe(const std::string &args)
{
    std::string action = strprintf("*%s*", args.c_str());
    chatWindow->chatInput(action);
}

void CommandHandler::handleRecord(const std::string &args)
{
    chatWindow->setRecordingFile(args);
}

void CommandHandler::handleToggle(const std::string &args)
{
    if (args.empty())
    {
        localChatTab->chatLog(chatWindow->getReturnTogglesChat() ?
                _("Return toggles chat.") : _("Message closes chat."));
        return;
    }

    std::string opt = args.substr(0, 1);

    if (opt == "1" ||
        opt == "y" || opt == "Y" ||
        opt == "t" || opt == "T")
    {
        localChatTab->chatLog(_("Return now toggles chat."));
        chatWindow->setReturnTogglesChat(true);
        return;
    }
    else if (opt == "0" ||
             opt == "n" || opt == "N" ||
             opt == "f" || opt == "F")
    {
        localChatTab->chatLog(_("Message now closes chat."));
        chatWindow->setReturnTogglesChat(false);
        return;
    }
    else
        localChatTab->chatLog(_("Options to /toggle are \"yes\", \"no\", \"true\", "
                    "\"false\", \"1\", \"0\"."));
}

void CommandHandler::handlePresent(const std::string &args)
{
    chatWindow->doPresent();
}
