/*
 *  The Mana Client
 *  Copyright (C) 2004-2010  The Mana World Development Team
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

#include "channelmanager.h"

#include "channel.h"

#include "utils/dtor.h"

ChannelManager::ChannelManager()
{
}

ChannelManager::~ChannelManager()
{
    delete_all(mChannels);
    mChannels.clear();
}

Channel *ChannelManager::findById(int id) const
{
    Channel *channel = 0;
    for (std::list<Channel*>::const_iterator itr = mChannels.begin(),
                                             end = mChannels.end();
         itr != end;
         itr++)
    {
        Channel *c = (*itr);
        if (c->getId() == id)
        {
            channel = c;
            break;
        }
    }
    return channel;
}

Channel *ChannelManager::findByName(const std::string &name) const
{
    Channel *channel = 0;
    if (!name.empty())
    {
        for (std::list<Channel*>::const_iterator itr = mChannels.begin(),
                                                 end = mChannels.end();
             itr != end;
             itr++)
        {
            Channel *c = (*itr);
            if (c->getName() == name)
            {
                channel = c;
                break;
            }
        }
    }
    return channel;
}

void ChannelManager::addChannel(Channel *channel)
{
    mChannels.push_back(channel);
}

void ChannelManager::removeChannel(Channel *channel)
{
    mChannels.remove(channel);
    delete channel;
}
