/*
 *  The Mana Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <string>

class ChatTab;

extern ChatTab *localChatTab;

#define BOOLEAN_OPTIONS _("Options to /%s are \"yes\", \"no\", \"true\", "\
"\"false\", \"1\", \"0\".")

/**
 * A class to parse and handle user commands
 */
class CommandHandler
{
    public:
        CommandHandler();

        ~CommandHandler() {}

        /**
         * Parse and handle the given command.
         */
        void handleCommand(const std::string &command, ChatTab *tab = localChatTab);

        static char parseBoolean(const std::string &value);

    protected:
        friend class ChatTab;
        friend class WhisperTab;

        /**
         * Handle an announce command.
         */
        void handleAnnounce(const std::string &args, ChatTab *tab);

        /**
         * Handle a help command.
         */
        void handleHelp(const std::string &args, ChatTab *tab);

        /**
         * Handle a where command.
         */
        void handleWhere(const std::string &args, ChatTab *tab);

        /**
         * Handle a who command.
         */
        void handleWho(const std::string &args, ChatTab *tab);

        /**
         * Handle a msg command.
         */
        void handleMsg(const std::string &args, ChatTab *tab);

        /**
         * Handle a msg tab request.
         */
        void handleQuery(const std::string &args, ChatTab *tab);

        /**
         * Handle a join command.
         */
        void handleJoin(const std::string &args, ChatTab *tab);

        /**
         * Handle a listchannels command.
         */
        void handleListChannels(const std::string &args, ChatTab *tab);

        /**
         * Handle a clear command.
         */
        void handleClear(const std::string &args, ChatTab *tab);

        /**
         * Handle a createparty command.
         */
        void handleCreateParty(const std::string &args, ChatTab *tab);

        /**
         * Handle a party command.
         */
        void handleParty(const std::string &args, ChatTab *tab);

        /**
         * Handle a me command.
         */
        void handleMe(const std::string &args, ChatTab *tab);

        /**
         * Handle a record command.
         */
        void handleRecord(const std::string &args, ChatTab *tab);

        /**
         * Handle a toggle command.
         */
        void handleToggle(const std::string &args, ChatTab *tab);

        /**
         * Handle a present command.
         */
        void handlePresent(const std::string &args, ChatTab *tab);

        /**
         * Handle an ignore command.
         */
        void handleIgnore(const std::string &args, ChatTab *tab);

        /**
         * Handle an unignore command.
         */
        void handleUnignore(const std::string &args, ChatTab *tab);

        /**
         * Handle away command.
         */
        void handleAway(const std::string &args, ChatTab *tab);

        /*
         * Handle showip command.
         */
        void handleShowIp(const std::string &args, ChatTab *tab);
};

extern CommandHandler *commandHandler;

#endif // COMMANDHANDLER_H
