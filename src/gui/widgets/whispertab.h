/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
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

#ifndef WHISPERTAB_H
#define WHISPERTAB_H

#include "chattab.h"

class Channel;

/**
 * A tab for whispers from a single player.
 */
class WhisperTab : public ChatTab
{
    public:
        const std::string &getNick() const { return mNick; }

        void showHelp();

        bool handleCommand(const std::string &type,
                           const std::string &args);

    protected:
        friend class ChatWindow;

        /**
         * Constructor.
         *
         * @param nick  the name of the player this tab is whispering to
         */
        WhisperTab(const std::string &nick);

        ~WhisperTab();

        void handleInput(const std::string &msg);

        void handleCommand(const std::string &msg);

    private:
        std::string mNick;
};

#endif // CHANNELTAB_H
