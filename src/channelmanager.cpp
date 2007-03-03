/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
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
 *
 *  $Id$
 */

#include <list>

#include "channelmanager.h"
#include "channel.h"

#include "utils/dtor.h"

ChannelManager::ChannelManager()
{

}

ChannelManager::~ChannelManager()
{
    for_each(mChannels.begin(), mChannels.end(), make_dtor(mChannels));
    mChannels.clear();
}

Channel* ChannelManager::findById(int id)
{
    Channel* channel;
    for(std::list<Channel*>::iterator itr = mChannels.begin();
        itr != mChannels.end();
        itr++)
    {
        channel = (*itr);
        if(channel->getId() == id)
        {
            return channel;
        }
    }
    return NULL;
}

Channel* ChannelManager::findByName(std::string name)
{
    Channel* channel;
    if(name != "")
    {
        for(std::list<Channel*>::iterator itr = mChannels.begin();
            itr != mChannels.end();
            itr++)
        {
            channel = (*itr);
            if(channel->getName() == name)
            {
                return channel;
            }
        }
    }
    return NULL;
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
