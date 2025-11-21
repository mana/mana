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

#pragma once

#include <string>

class ChatTab;

extern ChatTab *localChatTab;

std::string booleanOptionInstructions(const char *command);

/**
 * A class to parse and handle user commands
 */
class CommandHandler
{
    public:
        CommandHandler() = default;
        ~CommandHandler() = default;

        /**
         * Parse and handle the given command.
         */
        static void handleCommand(const std::string &command, ChatTab *tab = localChatTab);

        static int parseBoolean(const std::string &value);

    protected:
        friend class ChatTab;
        friend class WhisperTab;

        /**
         * Handle a help command.
         */
        static void handleHelp(const std::string &args, ChatTab *tab);

        /**
         * Handle a where command.
         */
        static void handleWhere(const std::string &args, ChatTab *tab);

        /**
         * Handle a who command.
         */
        static void handleWho(const std::string &args, ChatTab *tab);

        /**
         * Handle a msg command.
         */
        static void handleMsg(const std::string &args, ChatTab *tab);

        /**
         * Handle a msg tab request.
         */
        static void handleQuery(const std::string &args, ChatTab *tab);

        /**
         * Handle a join command.
         */
        static void handleJoin(const std::string &args, ChatTab *tab);

        /**
         * Handle a listchannels command.
         */
        static void handleListChannels(const std::string &args, ChatTab *tab);

        /**
         * Handle a clear command.
         */
        static void handleClear(const std::string &args, ChatTab *tab);

        /**
         * Handle a createparty command.
         */
        static void handleCreateParty(const std::string &args, ChatTab *tab);

        /**
         * Handle a party command.
         */
        static void handleParty(const std::string &args, ChatTab *tab);

        /**
         * Handle a me command.
         */
        static void handleMe(const std::string &args, ChatTab *tab);

        /**
         * Handle a record command.
         */
        static void handleRecord(const std::string &args, ChatTab *tab);

        /**
         * Handle a toggle command.
         */
        static void handleToggle(const std::string &args, ChatTab *tab);

        /**
         * Handle a present command.
         */
        static void handlePresent(const std::string &args, ChatTab *tab);

        /**
         * Handle an ignore command.
         */
        static void handleIgnore(const std::string &args, ChatTab *tab);

        /**
         * Handle an unignore command.
         */
        static void handleUnignore(const std::string &args, ChatTab *tab);

        /**
         * Handle away command.
         */
        static void handleAway(const std::string &args, ChatTab *tab);

        /*
         * Handle showip command.
         */
        static void handleShowIp(const std::string &args, ChatTab *tab);
};

extern CommandHandler *commandHandler;
