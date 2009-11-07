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

GuildMember::GuildMember(int guildId, int id, const std::string &name):
        mName(name), mId(id), mOnline(false)
{
    mGuild = Guild::getGuild(guildId);
};

GuildMember::GuildMember(int guildId, int id):
        mId(id), mOnline(false)
{
    mGuild = Guild::getGuild(guildId);
};

GuildMember::GuildMember(int guildId, const std::string &name):
        mName(name), mId(0), mOnline(false)
{
    mGuild = Guild::getGuild(guildId);
};

Guild::GuildMap Guild::guilds;

Guild::Guild(short id):
    mId(id),
    mCanInviteUsers(false)
{
    guilds[id] = this;
}

void Guild::addMember(GuildMember *member)
{
    if (member->mGuild > 0 && member->mGuild != this)
    {
        throw "Member in another guild!";
    }

    if (!isMember(member))
    {
        mMembers.push_back(member);
        member->mGuild = this;
    }
}

GuildMember *Guild::getMember(int id)
{
    MemberList::iterator itr = mMembers.begin(),
                               itr_end = mMembers.end();
    while(itr != itr_end)
    {
        if((*itr)->mId == id)
        {
            return (*itr);
        }
        ++itr;
    }

    return NULL;
}

GuildMember *Guild::getMember(std::string name)
{
    MemberList::iterator itr = mMembers.begin(),
                               itr_end = mMembers.end();
    while(itr != itr_end)
    {
        if((*itr)->mName == name)
        {
            return (*itr);
        }
        ++itr;
    }

    return NULL;
}

void Guild::removeMember(GuildMember *member)
{
    MemberList::iterator itr = mMembers.begin(),
                               itr_end = mMembers.end();
    while(itr != itr_end)
    {
        if((*itr)->mId == member->mId && (*itr)->mName == member->mName)
        {
            mMembers.erase(itr);
        }
        ++itr;
    }
}

void Guild::removeMember(int id)
{
    MemberList::iterator itr = mMembers.begin(),
                               itr_end = mMembers.end();
    while(itr != itr_end)
    {
        if((*itr)->mId == id)
        {
            mMembers.erase(itr);
        }
        ++itr;
    }
}

void Guild::removeMember(const std::string &name)
{
    MemberList::iterator itr = mMembers.begin(),
                               itr_end = mMembers.end();
    while(itr != itr_end)
    {
        if((*itr)->mName == name)
        {
            mMembers.erase(itr);
        }
        ++itr;
    }
}

std::string Guild::getElementAt(int index)
{
    GuildMember *m = mMembers[index];
    if (m->mOnline)
    {
        return "* " + m->mName;
    }
    else
    {
        return m->mName;
    }
}

void Guild::setRights(short rights)
{
    // to invite, rights must be greater than 0
    if (rights > 0)
    {
        mCanInviteUsers = true;
    }
}

bool Guild::isMember(GuildMember *member) const
{
    if (member->mGuild > 0 && member->mGuild != this)
        return false;

    MemberList::const_iterator itr = mMembers.begin(),
                                     itr_end = mMembers.end();
    while (itr != itr_end)
    {
        if ((*itr)->mId == member->mId && (*itr)->mName == member->mName)
        {
            return true;
        }
        ++itr;
    }

    return false;
}

bool Guild::isMember(int id) const
{
    MemberList::const_iterator itr = mMembers.begin(),
                                     itr_end = mMembers.end();
    while (itr != itr_end)
    {
        if ((*itr)->mId == id)
        {
            return true;
        }
        ++itr;
    }

    return false;
}

bool Guild::isMember(const std::string &name) const
{
    MemberList::const_iterator itr = mMembers.begin(),
                                     itr_end = mMembers.end();
    while (itr != itr_end)
    {
        if ((*itr)->mName == name)
        {
            return true;
        }
        ++itr;
    }

    return false;
}

Guild *Guild::getGuild(int id)
{
    GuildMap::iterator it = guilds.find(id);
    if (it != guilds.end())
        return it->second;

    return new Guild(id);
}
