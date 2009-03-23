/*
 *  The Mana World
 *  Copyright 2008 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "commandhandler.h"
#include "channelmanager.h"
#include "channel.h"
#include "game.h"
#include "localplayer.h"

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
        chatWindow->chatLog("Unknown command");
    }
}

void CommandHandler::handleAnnounce(const std::string &args)
{
#ifdef TMWSERV_SUPPORT
    Net::ChatServer::announce(args);
#else
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(0x0099);
    outMsg.writeInt16(args.length() + 4);
    outMsg.writeString(args, args.length());
#endif
}

void CommandHandler::handleHelp(const std::string &args)
{
    if (args == "")
    {
        chatWindow->chatLog(_("-- Help --"));
        chatWindow->chatLog(_("/help > Display this help."));

        chatWindow->chatLog(_("/where > Display map name"));
        chatWindow->chatLog(_("/who > Display number of online users"));
        chatWindow->chatLog(_("/me > Tell something about yourself"));

        chatWindow->chatLog(_("/msg > Send a private message to a user"));
        chatWindow->chatLog(_("/whisper > Alias of msg"));
        chatWindow->chatLog(_("/w > Alias of msg"));

#ifdef TMWSERV_SUPPORT
        chatWindow->chatLog(_("/list > Display all public channels"));
        chatWindow->chatLog(_("/users > Lists the users in the current channel"));
        chatWindow->chatLog(_("/join > Join or create a channel"));
        chatWindow->chatLog(_("/topic > Set the topic of the current channel"));
        chatWindow->chatLog(_("/quit > Leave a channel"));
        chatWindow->chatLog(_("/clear > Clears this window"));
        chatWindow->chatLog(_("/op > Make a user a channel operator"));
        chatWindow->chatLog(_("/kick > Kick a user from the channel"));

        chatWindow->chatLog(_("/party > Invite a user to party"));
#else
        chatWindow->chatLog(_("/party > Party-related commands"));
#endif

        chatWindow->chatLog(_("/record > Start recording the chat to an external file"));
        chatWindow->chatLog(_("/toggle > Determine whether <return> toggles the chat log"));
        chatWindow->chatLog(_("/present > Get list of players present (sent to chat log, if logging)"));

        chatWindow->chatLog(_("/announce > Global announcement (GM only)"));

        chatWindow->chatLog(_("For more information, type /help <command>"));
    }
    else if (args == "announce")
    {
        chatWindow->chatLog(_("Command: /announce <msg>"));
        chatWindow->chatLog(_("*** only available to a GM ***"));
        chatWindow->chatLog(_("This command sends the message <msg> to "
                            "all players currently online."));
    }
    else if (args == "clear")
    {
        chatWindow->chatLog(_("Command: /clear"));
        chatWindow->chatLog(_("This command clears the chat log of previous chat."));
    }
    else if (args == "help")
    {
        chatWindow->chatLog(_("Command: /help"));
        chatWindow->chatLog(_("This command displays a list of all commands available."));
        chatWindow->chatLog(_("Command: /help <command>"));
        chatWindow->chatLog(_("This command displays help on <command>."));
    }
    else if (args == "join")
    {
        chatWindow->chatLog(_("Command: /join <channel>"));
        chatWindow->chatLog(_("This command makes you enter <channel>."));
        chatWindow->chatLog(_("If <channel> doesn't exist, it's created."));
    }
    else if (args == "kick")
    {
        chatWindow->chatLog(_("Command: /kick <nick>"));
        chatWindow->chatLog(_("This command makes <nick> leave the channel."));
        chatWindow->chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
    }
    else if (args == "list")
    {
        chatWindow->chatLog(_("Command: /list"));
        chatWindow->chatLog(_("This command shows a list of all channels."));
    }
    else if (args == "me")
    {
        chatWindow->chatLog(_("Command: /me <message>"));
        chatWindow->chatLog(_("This command tell others you are (doing) <msg>."));
    }
    else if (args == "msg" || args == "whisper" || args == "w")
    {
        chatWindow->chatLog(_("Command: /msg <nick> <message>"));
        chatWindow->chatLog(_("Command: /whisper <nick> <message>"));
        chatWindow->chatLog(_("Command: /w <nick> <message>"));
        chatWindow->chatLog(_("This command sends the text <message> to <nick>."));
        chatWindow->chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
    }
    else if (args == "op")
    {
        chatWindow->chatLog(_("Command: /op <nick>"));
        chatWindow->chatLog(_("This command makes <nick> a channel operator."));
        chatWindow->chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
        chatWindow->chatLog(_("Channel operators can kick and op other users "
                            "from the channel."));
    }
#ifdef TMWSERV_SUPPORT
    else if (args == "party")
    {
        chatWindow->chatLog(_("Command: /party <nick>"));
        chatWindow->chatLog(_("This command invites <nick> to party with you."));
        chatWindow->chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
#else
    else if (args.substr(0, 5) == "party")
    {
        playerParty->help(args);
#endif
    }
    else if (args == "present")
    {
        chatWindow->chatLog(_("Command: /present"));
        chatWindow->chatLog(_("This command gets a list of players within hearing and "
                  "sends it to either the record log if recording, or the chat "
                  "log otherwise."));
    }
    else if (args == "quit")
    {
        chatWindow->chatLog(_("Command: /quit"));
        chatWindow->chatLog(_("This command leaves the current channel."));
        chatWindow->chatLog(_("If you're the last person in the channel, it will be deleted."));
    }
    else if (args == "record")
    {
        chatWindow->chatLog(_("Command: /record <filename>"));
        chatWindow->chatLog(_("This command starts recording the chat log to the file "
                  "<filename>."));
        chatWindow->chatLog(_("Command: /record"));
        chatWindow->chatLog(_("This command finishes a recording session."));
    }
    else if (args == "toggle")
    {
        chatWindow->chatLog(_("Command: /toggle <state>"));
        chatWindow->chatLog(_("This command sets whether the return key should toggle the "
                  "chat log, or whether the chat log turns off automatically."));
        chatWindow->chatLog(_("<state> can be one of \"1\", \"yes\", \"true\" to "
                  "turn the toggle on, or \"0\", \"no\", \"false\" to turn the "
                  "toggle off."));
        chatWindow->chatLog(_("Command: /toggle"));
        chatWindow->chatLog(_("This command displays the return toggle status."));
    }
    else if (args == "topic")
    {
        chatWindow->chatLog(_("Command: /topic <message>"));
        chatWindow->chatLog(_("This command sets the topic to <message>."));
    }
    else if (args == "users")
    {
        chatWindow->chatLog(_("Command: /users <channel>"));
        chatWindow->chatLog(_("This command shows the users in <channel>."));
    }
    else if (args == "where")
    {
        chatWindow->chatLog(_("Command: /where"));
        chatWindow->chatLog(_("This command displays the name of the current map."));
    }
    else if (args == "who")
    {
        chatWindow->chatLog(_("Command: /who"));
        chatWindow->chatLog(_("This command displays the number of players currently "
                            "online."));
    }
    else
    {
        chatWindow->chatLog(_("Unknown command."));
        chatWindow->chatLog(_("Type /help for a list of commands."));
    }
}

void CommandHandler::handleWhere()
{
    // TODO: add position
    chatWindow->chatLog(map_path, BY_SERVER);
}

void CommandHandler::handleWho()
{
#ifdef TMWSERV_SUPPORT
    //TODO
#else
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(0x00c1);
#endif
}

void CommandHandler::handleMsg(const std::string &args)
{
#ifdef TMWSERV_SUPPORT
    std::string::size_type pos = args.find(' ');
    std::string recipient(args, 0, pos);
    std::string text(args, pos+1);
    Net::ChatServer::privMsg(recipient, text);
#else
    std::string recvnick = "";
    std::string msg = "";

    if (args.substr(0, 1) == "\"")
    {
        const std::string::size_type pos = args.find('"', 1);
        if (pos != std::string::npos)
        {
            recvnick = args.substr(1, pos - 1);
            msg = args.substr(pos + 2, args.length());
        }
    }
    else
    {
        const std::string::size_type pos = msg.find(" ");
        if (pos != std::string::npos)
        {
            recvnick = args.substr(0, pos);
            msg = args.substr(pos + 1, args.length());
        }
    }

    trim(msg);

    std::string playerName = player_node->getName();
    std::string tempNick = recvnick;

    toLower(playerName);
    toLower(tempNick);

    if (tempNick.compare(playerName) == 0 || args.empty())
        return;

    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_CHAT_WHISPER);
    outMsg.writeInt16(msg.length() + 28);
    outMsg.writeString(recvnick, 24);
    outMsg.writeString(msg, msg.length());

    chatWindow->chatLog(strprintf(_("Whispering to %s: %s"),
                        recvnick.c_str(), msg.c_str()),
                        BY_PLAYER);
#endif
}

void CommandHandler::handleClear()
{
    chatWindow->clearTab(chatWindow->getFocused());
}

#ifdef TMWSERV_SUPPORT

void CommandHandler::handleJoin(const std::string &args)
{
    std::string::size_type pos = args.find(' ');
    std::string name(args, 0, pos);
    std::string password(args, pos+1);
    chatWindow->chatLog("Requesting to join channel " + name);
    Net::ChatServer::enterChannel(name, password);
}

void CommandHandler::handleListChannels()
{
    Net::ChatServer::getChannelList();
}

void CommandHandler::handleListUsers()
{
    Net::ChatServer::getUserList(chatWindow->getFocused());
}

void CommandHandler::handleTopic(const std::string &args)
{
    if (Channel *channel = channelManager->findByName(chatWindow->getFocused()))
    {
        Net::ChatServer::setChannelTopic(channel->getId(), args);
    }
    else
    {
        chatWindow->chatLog("Unable to set this channel's topic", BY_CHANNEL);
    }
}

void CommandHandler::handleQuit()
{
    if (Channel *channel = channelManager->findByName(chatWindow->getFocused()))
    {
        Net::ChatServer::quitChannel(channel->getId());
    }
    else
    {
        chatWindow->chatLog("Unable to quit this channel", BY_CHANNEL);
    }
}

void CommandHandler::handleOp(const std::string &args)
{
    if (Channel *channel = channelManager->findByName(chatWindow->getFocused()))
    {
        // set the user mode 'o' to op a user
        if (args != "")
        {
            Net::ChatServer::setUserMode(channel->getId(), args, 'o');
        }
    }
    else
    {
        chatWindow->chatLog("Unable to set this user's mode", BY_CHANNEL);
    }
}

void CommandHandler::handleKick(const std::string &args)
{
    if (Channel *channel = channelManager->findByName(chatWindow->getFocused()))
    {
        if (args != "")
        {
            Net::ChatServer::kickUser(channel->getId(), args);
        }
    }
    else
    {
        chatWindow->chatLog("Unable to kick user", BY_CHANNEL);
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
        chatWindow->chatLog(_("Unknown party command... Type \"/help\" party for more "
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
            chatWindow->chatLog(_("The current party prefix is ") + std::string(temp));
        }
        else if (rest.length() != 1)
        {
            chatWindow->chatLog(_("Party prefix must be one character long."));
        }
        else
        {
            if (rest == "/")
            {
                chatWindow->chatLog(_("Cannot use a '/' as the prefix."));
            }
            else
            {
                chatWindow->setPartyPrefix(rest.at(0));
                chatWindow->chatLog(_("Changing prefix to ") + rest);
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
    chatWindow->chatSend(action);
    //std::stringstream actionStr;
    //actionStr << "*" << args << "*";
    //chatWindow->chatSend(actionStr.str());
}

void CommandHandler::handleRecord(const std::string &args)
{
    chatWindow->setRecordingFile(args);
}

void CommandHandler::handleToggle(const std::string &args)
{
    if (args.empty())
    {
        chatWindow->chatLog(chatWindow->getReturnTogglesChat() ?
                _("Return toggles chat.") : _("Message closes chat."));
        return;
    }

    std::string opt = args.substr(0, 1);

    if (opt == "1" ||
        opt == "y" || opt == "Y" ||
        opt == "t" || opt == "T")
    {
        chatWindow->chatLog(_("Return now toggles chat."));
        chatWindow->setReturnTogglesChat(true);
        return;
    }
    else if (opt == "0" ||
             opt == "n" || opt == "N" ||
             opt == "f" || opt == "F")
    {
        chatWindow->chatLog(_("Message now closes chat."));
        chatWindow->setReturnTogglesChat(false);
        return;
    }
    else
        chatWindow->chatLog(_("Options to /toggle are \"yes\", \"no\", \"true\", "
                    "\"false\", \"1\", \"0\"."));
}

void CommandHandler::handlePresent(const std::string &args)
{
    chatWindow->doPresent();
}
