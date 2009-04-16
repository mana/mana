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

#ifndef CHANNELTAB_H
#define CHANNELTAB_H

#include "chattab.h"

class Channel;

/**
 * A tab for a chat channel.
 */
class ChannelTab : public ChatTab
{
    public:

        Channel *getChannel() const { return mChannel; }

        void showHelp();

        bool handleCommand(const std::string &type,
                           const std::string &args);

    protected:
        friend class Channel;

        /**
         * Constructor.
         */
        ChannelTab(Channel *channel);

        /**
         * Destructor.
         */
        ~ChannelTab();

        void handleInput(const std::string &msg);

    private:
        Channel *mChannel;
};

#endif // CHANNELTAB_H
