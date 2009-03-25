/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
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

#include "guild.h"

Guild::Guild(short id, short rights):
    mId(id),
    mCanInviteUsers(false)
{
    // to invite, rights must be greater than 0
    if (rights > 0)
        mCanInviteUsers = true;
}

void Guild::addMember(const std::string &name)
{
    if (!isMember(name))
    {
        mMembers.push_back(name);
    }
}

void Guild::removeMember(const std::string &name)
{
    std::vector<std::string>::iterator itr = mMembers.begin(),
                                       itr_end = mMembers.end();
    while(itr != itr_end)
    {
        if((*itr) == name)
        {
            mMembers.erase(itr);
        }
        ++itr;
    }
}

bool Guild::isMember(const std::string &name)
{
    std::vector<std::string>::iterator itr = mMembers.begin(),
                                       itr_end = mMembers.end();
    while(itr != itr_end)
    {
        if((*itr) == name)
        {
            return true;
        }
        ++itr;
    }

    return false;
}
