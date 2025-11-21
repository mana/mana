/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "commandhandler.h"

#include "actorspritemanager.h"
#include "channelmanager.h"
#include "configuration.h"
#include "game.h"
#include "localplayer.h"
#include "playerrelations.h"

#include "gui/widgets/chattab.h"

#include "net/chathandler.h"
#include "net/net.h"
#include "net/partyhandler.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

std::string booleanOptionInstructions(const char *command)
{
    return strprintf(_("Options to /%s are \"yes\", \"no\", \"true\", \"false\", \"1\", \"0\"."),
                     command);
}

void CommandHandler::handleCommand(const std::string &command, ChatTab *tab)
{
    std::string::size_type pos = command.find(' ');
    std::string type(command, 0, pos);
    std::string args(command, pos == std::string::npos ?
                                     command.size() : pos + 1);
    trim(args);

    if (type == "help") // Do help before tabs so they can't override it
    {
        handleHelp(args, tab);
    }
    else if (tab->handleCommand(type, args))
    {
        // Nothing to do
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
    else if (type == "ignore")
    {
        handleIgnore(args, tab);
    }
    else if (type == "unignore")
    {
        handleUnignore(args, tab);
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
    else if (type == "createparty")
    {
        handleCreateParty(args, tab);
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
    else if (type == "showip" && Net::getNetworkType() == ServerType::TmwAthena)
    {
        handleShowIp(args, tab);
    }
    else if (type == "away")
    {
        handleAway(args, tab);
    }
    else
    {
        tab->chatLog(_("Unknown command."));
    }
}

int CommandHandler::parseBoolean(const std::string &value)
{
    switch (value.empty() ? 0 : value[0]) {
    case '1':
    case 'y': case 'Y':
    case 't': case 'T':
        return 1;

    case '0':
    case 'n': case 'N':
    case 'f': case 'F':
        return 0;

    default:
        return -1;
    }
}

void CommandHandler::handleHelp(const std::string &args, ChatTab *tab)
{
    if (args.empty())
    {
        tab->chatLog(_("-- Help --"));
        tab->chatLog(_("/help > Display this help"));

        tab->chatLog(_("/where > Display map name"));

        if (Net::getChatHandler()->whoSupported())
            tab->chatLog(_("/who > Display number of online users"));

        tab->chatLog(_("/me > Tell something about yourself"));

        tab->chatLog(_("/clear > Clears this window"));

        tab->chatLog(_("/msg > Send a private message to a user"));
        tab->chatLog(_("/whisper > Alias of msg"));
        tab->chatLog(_("/w > Alias of msg"));
        tab->chatLog(_("/query > Makes a tab for private messages "
                       "with another user"));
        tab->chatLog(_("/q > Alias of query"));

        tab->chatLog(_("/away > Tell the other whispering players "
                       "you're away from keyboard."));

        tab->chatLog(_("/ignore > ignore a player"));
        tab->chatLog(_("/unignore > stop ignoring a player"));

        tab->chatLog(_("/list > Display all public channels"));
        tab->chatLog(_("/join > Join or create a channel"));

        tab->chatLog(_("/createparty > Create a new party"));
        tab->chatLog(_("/party > Invite a user to party"));

        tab->chatLog(_("/record > Start recording the chat "
                       "to an external file"));
        tab->chatLog(_("/toggle > Determine whether <return> "
                       "toggles the chat log"));
        tab->chatLog(_("/present > Get list of players present "
                       "(sent to chat log, if logging)"));

        tab->showHelp(); // Allow the tab to show it's help

        tab->chatLog(_("For more information, type /help <command>."));
    }
    else if (args == "help") // Do this before tabs so they can't change it
    {
        tab->chatLog(_("Command: /help"));
        tab->chatLog(_("This command displays a list "
                       "of all commands available."));
        tab->chatLog(_("Command: /help <command>"));
        tab->chatLog(_("This command displays help on <command>."));
    }
    else if (tab->handleCommand("help", args))
    {
        // Nothing to do
    }
    else if (args == "clear")
    {
        tab->chatLog(_("Command: /clear"));
        tab->chatLog(_("This command clears the chat log of previous chat."));
    }
    else if (args == "ignore")
    {
        tab->chatLog(_("Command: /ignore <player>"));
        tab->chatLog(_("This command ignores the given player regardless of "
                       "current relations."));
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
        tab->chatLog(_("This command tries to make a tab for whispers between "
                       "you and <nick>."));
    }
    else if (args == "away")
    {
        tab->chatLog(_("Command: /away <afk reason>"));
        tab->chatLog(_("This command tells "
                       "you're away from keyboard with the given reason."));
        tab->chatLog(_("Command: /away"));
        tab->chatLog(_("This command clears the away status and message."));
    }
    else if (args == "createparty")
    {
        tab->chatLog(_("Command: /createparty <name>"));
        tab->chatLog(_("This command creates a new party called <name>."));
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
    else if (args == "unignore")
    {
        tab->chatLog(_("Command: /unignore <player>"));
        tab->chatLog(_("This command stops ignoring the given player if they "
                       "are being ignored"));
    }
    else if (args == "where")
    {
        tab->chatLog(_("Command: /where"));
        tab->chatLog(_("This command displays the name of the current map."));
    }
    else if (args == "who" && Net::getChatHandler()->whoSupported())
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
    where << Game::instance()->getCurrentMapName() << ", coordinates: "
          << local_player->getTileX() << ", "
          << local_player->getTileY();

    tab->chatLog(where.str(), BY_SERVER);
}

void CommandHandler::handleWho(const std::string &args, ChatTab *tab)
{
    Net::getChatHandler()->who();
}

void CommandHandler::handleMsg(const std::string &args, ChatTab *tab)
{
    std::string recvnick;
    std::string msg;

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
        const std::string::size_type pos = args.find(' ');
        if (pos != std::string::npos)
        {
            recvnick = args.substr(0, pos);
            if (pos + 1 < args.length())
                msg = args.substr(pos + 1, args.length());
        }
        else
        {
            recvnick = std::string(args);
            msg.clear();
        }
    }

    trim(msg);

    if (!msg.empty())
    {
        std::string playerName = local_player->getName();
        std::string tempNick = recvnick;

        toLower(playerName);
        toLower(tempNick);

        if (tempNick == playerName || args.empty())
            return;

        chatWindow->whisper(recvnick, msg, BY_PLAYER);
    }
    else
        tab->chatLog(_("Cannot send empty whispers!"), BY_SERVER);
}

void CommandHandler::handleQuery(const std::string &args, ChatTab *tab)
{
    if (args.empty())
    {
        tab->chatLog(_("No <nick> was given."), BY_SERVER);
        return;
    }

    if (args.length() > 1 && args[0] == '\"' && args[args.length() - 1] == '\"')
    {
        if (chatWindow->addWhisperTab(args.substr(1, args.length() - 2), true))
            return;
    }
    else if (chatWindow->addWhisperTab(args, true))
    {
        return;
    }

    tab->chatLog(strprintf(_("Cannot create a whisper tab for nick \"%s\"! "
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
    std::string password(args, pos + 1);
    tab->chatLog(strprintf(_("Requesting to join channel %s."), name.c_str()));
    Net::getChatHandler()->enterChannel(name, password);
}

void CommandHandler::handleListChannels(const std::string &args, ChatTab *tab)
{
    Net::getChatHandler()->channelList();
}

void CommandHandler::handleCreateParty(const std::string &args, ChatTab *tab)
{
    if (args.empty())
        tab->chatLog(_("Party name is missing."), BY_SERVER);
    else
        Net::getPartyHandler()->create(args);
}

void CommandHandler::handleParty(const std::string &args, ChatTab *tab)
{
    if (args.empty())
        tab->chatLog(_("Please specify a name."), BY_SERVER);
    else
        Net::getPartyHandler()->invite(args);
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
        tab->chatLog(config.returnTogglesChat ? _("Return toggles chat.")
                                              : _("Message closes chat."));
        return;
    }

    switch (parseBoolean(args))
    {
        case 1:
            tab->chatLog(_("Return now toggles chat."));
            config.returnTogglesChat = true;
            return;
        case 0:
            tab->chatLog(_("Message now closes chat."));
            config.returnTogglesChat = false;
            return;
        case -1:
            tab->chatLog(booleanOptionInstructions("toggle"));
    }
}

void CommandHandler::handleShowIp(const std::string &args, ChatTab *tab)
{
    if (args.empty())
    {
        tab->chatLog(local_player->getShowIp() ?
                _("Show IP: On") : _("Show IP: Off"));
        return;
    }

    switch (parseBoolean(args))
    {
        case 0:
            tab->chatLog(_("Show IP: Off"));
            local_player->setShowIp(false);
            break;
        case 1:
            tab->chatLog(_("Show IP: On"));
            local_player->setShowIp(true);
            break;
        case -1:
            tab->chatLog(booleanOptionInstructions("showip"));
            return;
    }

    actorSpriteManager->updatePlayerNames();
}

void CommandHandler::handlePresent(const std::string &args, ChatTab *tab)
{
    chatWindow->doPresent();
}

void CommandHandler::handleIgnore(const std::string &args, ChatTab *tab)
{
    if (args.empty())
    {
        tab->chatLog(_("Please specify a name."), BY_SERVER);
        return;
    }

    if (player_relations.getRelation(args) == PlayerRelation::Ignored)
    {
        tab->chatLog(_("Player already ignored!"), BY_SERVER);
        return;
    }
    else
        player_relations.setRelation(args, PlayerRelation::Ignored);

    if (player_relations.getRelation(args) == PlayerRelation::Ignored)
        tab->chatLog(_("Player successfully ignored!"), BY_SERVER);
    else
        tab->chatLog(_("Player could not be ignored!"), BY_SERVER);
}

void CommandHandler::handleUnignore(const std::string &args, ChatTab *tab)
{
    if (args.empty())
    {
        tab->chatLog(_("Please specify a name."), BY_SERVER);
        return;
    }

    if (player_relations.getRelation(args) == PlayerRelation::Ignored)
        player_relations.removePlayer(args);
    else
    {
        tab->chatLog(_("Player wasn't ignored!"), BY_SERVER);
        return;
    }

    if (player_relations.getRelation(args) != PlayerRelation::Ignored)
        tab->chatLog(_("Player no longer ignored!"), BY_SERVER);
    else
        tab->chatLog(_("Player could not be unignored!"), BY_SERVER);
}

void CommandHandler::handleAway(const std::string &args, ChatTab *tab)
{
    if (!args.empty())
        config.afkMessage = args;
    local_player->setAwayMode(!local_player->getAwayMode());
}
