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

    if (type == "help") // Do help before tabs so they can't override it
    {
        handleHelp(args, tab);
    }
    else if (tab->handleCommand(type, args))
    {
        // Nothing to do
    }
    else if (type == "announce")
    {
        handleAnnounce(args, tab);
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
    else if (type == "query" || type == "q")
    {
        handleQuery(args, tab);
    }
    else if (type == "join")
    {
        handleJoin(args, tab);
    }
    else if (type == "list")
    {
        handleListChannels(args, tab);
    }
    else if (type == "clear")
    {
        handleClear(args, tab);
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

char CommandHandler::parseBoolean(const std::string &value)
{
    std::string opt = value.substr(0, 1);

    if (opt == "1" ||
        opt == "y" || opt == "Y" ||
        opt == "t" || opt == "T")
        return 1;
    else if (opt == "0" ||
             opt == "n" || opt == "N" ||
             opt == "f" || opt == "F")
        return 0;
    else
        return -1;
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
        tab->chatLog(_("/help > Display this help"));

        tab->chatLog(_("/where > Display map name"));
        tab->chatLog(_("/who > Display number of online users"));
        tab->chatLog(_("/me > Tell something about yourself"));

        tab->chatLog(_("/clear > Clears this window"));

        tab->chatLog(_("/msg > Send a private message to a user"));
        tab->chatLog(_("/whisper > Alias of msg"));
        tab->chatLog(_("/w > Alias of msg"));
        tab->chatLog(_("/query > Makes a tab for private messages with another user"));
        tab->chatLog(_("/q > Alias of query"));

        tab->chatLog(_("/list > Display all public channels"));
        tab->chatLog(_("/join > Join or create a channel"));

        tab->chatLog(_("/party > Invite a user to party"));

        tab->chatLog(_("/record > Start recording the chat to an external file"));
        tab->chatLog(_("/toggle > Determine whether <return> toggles the chat log"));
        tab->chatLog(_("/present > Get list of players present (sent to chat log, if logging)"));

        tab->chatLog(_("/announce > Global announcement (GM only)"));

        tab->showHelp(); // Allow the tab to show it's help

        tab->chatLog(_("For more information, type /help <command>"));
    }
    else if (args == "help") // Do this before tabs so they can't change it
    {
        tab->chatLog(_("Command: /help"));
        tab->chatLog(_("This command displays a list of all commands available."));
        tab->chatLog(_("Command: /help <command>"));
        tab->chatLog(_("This command displays help on <command>."));
    }
    else if (tab->handleCommand("help", args))
    {
        // Nothing to do
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
    else if (args == "join")
    {
        tab->chatLog(_("Command: /join <channel>"));
        tab->chatLog(_("This command makes you enter <channel>."));
        tab->chatLog(_("If <channel> doesn't exist, it's created."));
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
    else if (args == "query" || args == "q")
    {
        tab->chatLog(_("Command: /query <nick>"));
        tab->chatLog(_("Command: /q <nick>"));
        tab->chatLog(_("This command tries to make a tab for whispers between"
                       "you and <nick>."));
    }
    else if (args == "party")
    {
        tab->chatLog(_("Command: /party <nick>"));
        tab->chatLog(_("This command invites <nick> to party with you."));
        tab->chatLog(_("If the <nick> has spaces in it, enclose it in "
                            "double quotes (\")."));
    }
    else if (args == "present")
    {
        tab->chatLog(_("Command: /present"));
        tab->chatLog(_("This command gets a list of players within hearing and "
                  "sends it to either the record log if recording, or the chat "
                  "log otherwise."));
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
    std::ostringstream where;
    where << map_path << ", coordinates: " << player_node->mX << ", " << player_node->mY;
    tab->chatLog(where.str(), BY_SERVER);
}

void CommandHandler::handleWho(const std::string &args, ChatTab *tab)
{
    Net::getChatHandler()->who();
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
        tab->chatLog(_("Cannot send empty whispers!"), BY_SERVER);
}

void CommandHandler::handleQuery(const std::string &args, ChatTab *tab) {
    if (chatWindow->addWhisperTab(args, true))
        return;

    tab->chatLog(strprintf(_("Cannot create a whisper tab for nick '%s'! "
            "It either already exists, or is you."), args.c_str()), BY_SERVER);
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

void CommandHandler::handleParty(const std::string &args, ChatTab *tab)
{
    if (args != "")
        player_node->inviteToParty(args);
    else
        tab->chatLog("Please specify a name.", BY_SERVER);
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

    char opt = parseBoolean(args);

    switch (opt)
    {
        case 1:
            tab->chatLog(_("Return now toggles chat."));
            chatWindow->setReturnTogglesChat(true);
            return;
        case 0:
            tab->chatLog(_("Message now closes chat."));
            chatWindow->setReturnTogglesChat(false);
            return;
        case -1:
            tab->chatLog(strprintf(BOOLEAN_OPTIONS, "toggle"));
    }
}

void CommandHandler::handlePresent(const std::string &args, ChatTab *tab)
{
    chatWindow->doPresent();
}
