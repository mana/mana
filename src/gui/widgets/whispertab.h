/*
 *  The Mana Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "chattab.h"

class Channel;

/**
 * A tab for whispers from a single player.
 */
class WhisperTab : public ChatTab
{
    public:
        const std::string &getNick() const { return mNick; }

        void showHelp() override;

        bool handleCommand(const std::string &type,
                           const std::string &args) override;

        void saveToLogFile(std::string &msg) override;

    protected:
        friend class ChatWindow;

        /**
         * Constructor.
         *
         * @param nick  the name of the player this tab is whispering to
         */
        WhisperTab(const std::string &nick);

        ~WhisperTab() override;

        void handleInput(const std::string &msg) override;

        void handleCommand(const std::string &msg) override;

        bool checkNotify(Own) const override
        { return true; }

    private:
        std::string mNick;
};
