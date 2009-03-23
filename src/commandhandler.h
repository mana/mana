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

#ifndef _TMW_COMMANDHANDLER_H
#define _TMW_COMMANDHANDLER_H

#include <string>

#ifdef EATHENA_SUPPORT
class Network;
#endif

/**
 * A class to parse and handle user commands
 */
class CommandHandler
{
    public:
        /**
         * Constructor
         */
#ifdef TMWSERV_SUPPORT
        CommandHandler();
#else
        CommandHandler(Network *network);
#endif

        /**
         * Destructor
         */
        ~CommandHandler() {}

        /**
         * Parse and handle the given command.
         */
        void handleCommand(const std::string &command);

    private:
        /**
         * Handle an announce command.
         */
        void handleAnnounce(const std::string &args);

        /**
         * Handle a help command.
         */
        void handleHelp(const std::string &args);

        /**
         * Handle a where command.
         */
        void handleWhere();

        /**
         * Handle a who command.
         */
        void handleWho();

        /**
         * Handle a msg command.
         */
        void handleMsg(const std::string &args);

        /**
         * Handle a join command.
         */
        void handleJoin(const std::string &args);

        /**
         * Handle a listchannels command.
         */
        void handleListChannels();

        /**
         * Handle a listusers command.
         */
        void handleListUsers();

        /**
         * Handle a topic command.
         */
        void handleTopic(const std::string &args);

        /**
         * Handle a quit command.
         */
        void handleQuit();

        /**
         * Handle a clear command.
         */
        void handleClear();

        /**
         * Handle a party command.
         */
        void handleParty(const std::string &args);

        /**
         * Handle a op command.
         */
        void handleOp(const std::string &args);

        /**
         * Handle a kick command.
         */
        void handleKick(const std::string &args);

        /**
         * Handle a me command.
         */
        void handleMe(const std::string &args);

        /**
         * Handle a record command.
         */
        void handleRecord(const std::string &args);

        /**
         * Handle a toggle command.
         */
        void handleToggle(const std::string &args);

        /**
         * Handle a present command.
         */
        void handlePresent(const std::string &args);

#ifdef EATHENA_SUPPORT
        Network *mNetwork;
#endif
};

extern CommandHandler *commandHandler;

#endif //_TMW_COMMANDHANDLER_H
