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
 *
 *  $Id$
 */


#include "commandhandler.h"
#include "channelmanager.h"
#include "channel.h"
#include "game.h"
#include "localplayer.h"
#include "gui/chat.h"
#include "net/chatserver/chatserver.h"
#include "net/gameserver/player.h"

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
    else if (type == "msg")
    {
        handleMsg(args);
    }
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
    else if (type == "admin")
    {
        Net::GameServer::Player::say("/" + args);
    }
    else if (type == "clear")
    {
        handleClear();
    }
    else if (type == "party")
    {
        handleParty(args);
    }
    else if (type == "op")
    {
        handleOp(args);
    }
    else
    {
        chatWindow->chatLog("Unknown command");
    }
}

void CommandHandler::handleAnnounce(const std::string &args)
{
    Net::ChatServer::announce(args);
}

void CommandHandler::handleHelp(const std::string &args)
{
    if (args == "")
    {
        chatWindow->chatLog("-- Help --");
        chatWindow->chatLog("/help > Display this help.");
        chatWindow->chatLog("/announce > Global announcement (GM only)");
        chatWindow->chatLog("/where > Display map name");
        chatWindow->chatLog("/who > Display number of online users");
        chatWindow->chatLog("/msg > Send a private message to a user");
        chatWindow->chatLog("/list > Display all public channels");
        chatWindow->chatLog("/users > Lists the users in the current channel");
        chatWindow->chatLog("/join > Join or create a channel");
        chatWindow->chatLog("/topic > Set the topic of the current channel");
        chatWindow->chatLog("/quit > Leave a channel");
        chatWindow->chatLog("/admin > Send a command to the server (GM only)");
        chatWindow->chatLog("/clear > Clears this window");
        chatWindow->chatLog("/party > Invite a user to party");
        chatWindow->chatLog("For more information, type /help <command>");
    }
    else if (args == "admin")
    {
        chatWindow->chatLog("Command: /admin <command>");
        chatWindow->chatLog("*** only available to a GM ***");
        chatWindow->chatLog("This command sends an admin command to the server.");
        chatWindow->chatLog("<command> can be:");
        chatWindow->chatLog("reload <db>");
        chatWindow->chatLog("warp <name> <map> <x> <y>");
        chatWindow->chatLog("item <name> <id> <quantity>");
        chatWindow->chatLog("drop <id> <quantity>");
        chatWindow->chatLog("money <name> <quantity>");
        chatWindow->chatLog("spawn <id> <quantity>");
        chatWindow->chatLog("goto <name>");
        chatWindow->chatLog("recall <name>");
        chatWindow->chatLog("ban <name> <duration>");
    }
    else if (args == "announce")
    {
        chatWindow->chatLog("Command: /announce <msg>");
        chatWindow->chatLog("*** only available to a GM ***");
        chatWindow->chatLog("This command sends the message <msg> to "
                            "all players currently online.");
    }
    else if (args == "clear")
    {
        chatWindow->chatLog("Command: /clear");
        chatWindow->chatLog("This command clears the chat log of previous chat.");
    }
    else if (args == "help")
    {
        chatWindow->chatLog("Command: /help");
        chatWindow->chatLog("This command displays a list of all commands available.");
        chatWindow->chatLog("Command: /help <command>");
        chatWindow->chatLog("This command displays help on <command>.");
    }
    else if (args == "join")
    {
        chatWindow->chatLog("Command: /join <channel>");
        chatWindow->chatLog("This command makes you enter <channel>.");
        chatWindow->chatLog("If <channel> doesn't exist, it's created.");
    }
    else if (args == "kick")
    {
        chatWindow->chatLog("Command: /kick <nick>");
        chatWindow->chatLog("This command makes <nick> leave the channel.");
        chatWindow->chatLog("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\").");
    }
    else if (args == "list")
    {
        chatWindow->chatLog("Command: /list");
        chatWindow->chatLog("This command shows a list of all channels.");
    }
    else if (args == "msg")
    {
        chatWindow->chatLog("Command: /msg <nick> <message>");
        chatWindow->chatLog("This command sends the text <message> to <nick>.");
        chatWindow->chatLog("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\").");
    }
    else if (args == "op")
    {
        chatWindow->chatLog("Command: /op <nick>");
        chatWindow->chatLog("This command makes <nick> a channel operator.");
        chatWindow->chatLog("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\").");
        chatWindow->chatLog("Channel operators can kick and op other users "
                            "from the channel.");
    }
    else if (args == "party")
    {
        chatWindow->chatLog("Command: /party <nick>");
        chatWindow->chatLog("This command invites <nick> to party with you.");
        chatWindow->chatLog("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\").");
    }
    else if (args == "quit")
    {
        chatWindow->chatLog("Command: /quit");
        chatWindow->chatLog("This command leaves the current channel.");
        chatWindow->chatLog("If you're the last person in the channel, it will be deleted.");
    }
    else if (args == "topic")
    {
        chatWindow->chatLog("Command: /topic <message>");
        chatWindow->chatLog("This command sets the topic to <message>.");
    }
    else if (args == "users")
    {
        chatWindow->chatLog("Command: /users <channel>");
        chatWindow->chatLog("This command shows the users in <channel>.");
    }
    else if (args == "where")
    {
        chatWindow->chatLog("Command: /where");
        chatWindow->chatLog("This command displays the name of the current map.");
    }
    else if (args == "who")
    {
        chatWindow->chatLog("Command: /who");
        chatWindow->chatLog("This command displays the number of players currently "
                            "online.");
    }
    else
    {
        chatWindow->chatLog("Unknown command.");
        chatWindow->chatLog("Type /help for a list of commands.");
    }
}

void CommandHandler::handleWhere()
{
    chatWindow->chatLog(map_path, BY_SERVER);
}

void CommandHandler::handleWho()
{

}

void CommandHandler::handleMsg(const std::string &args)
{
    std::string::size_type pos = args.find(' ');
    std::string recipient(args, 0, pos);
    std::string text(args, pos+1);
    Net::ChatServer::privMsg(recipient, text);
}

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

void CommandHandler::handleClear()
{
    chatWindow->clearTab(chatWindow->getFocused());
}

void CommandHandler::handleParty(const std::string &args)
{
    if (args != "")
    {
        player_node->inviteToParty(args);
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
