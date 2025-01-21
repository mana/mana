/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

class ChannelTab;

class Channel
{
    public:
        /**
         * Constructor.
         *
         * @param id           the id associated with the channel.
         * @param name         the name of the channel.
         * @param announcement a welcome message.
         */
        Channel(short id,
                const std::string &name,
                const std::string &announcement = std::string());

        ~Channel();

        /**
         * Get the id associated witht his channel.
         */
        int getId() const { return mId; }

         /**
         * Get this channel's name.
         */
        const std::string &getName() const
        { return mName; }

        /**
         * Get the announcement message for this channel.
         */
        const std::string &getAnnouncement() const
        { return mAnnouncement; }

        /**
         * Sets the name of the channel.
         */
        void setName(const std::string &channelName)
        { mName = channelName; }

        /**
         * Sets the announcement string of the channel.
         */
        void setAnnouncement(const std::string &channelAnnouncement)
        { mAnnouncement = channelAnnouncement; }

        ChannelTab *getTab() { return mTab; }

    private:
        unsigned short mId;
        std::string mName;
        std::string mAnnouncement;
        ChannelTab *mTab;
};
