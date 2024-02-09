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

#include "guild.h"

#include "actorspritemanager.h"

GuildMember::GuildMember(Guild *guild, int id, const std::string &name):
    Avatar(name), mId(id), mGuild(guild)
{
}

GuildMember::GuildMember(Guild *guild, const std::string &name):
    Avatar(name), mGuild(guild)
{
}

Guild::GuildMap Guild::guilds;

Guild::Guild(short id):
    mId(id)
{
    guilds[id] = this;
}

GuildMember *Guild::addMember(int id, const std::string &name)
{
    GuildMember *m;
    if ((m = getMember(id)))
    {
        return m;
    }

    m = new GuildMember(this, id, name);

    mMembers.push_back(m);

    return m;
}

GuildMember *Guild::addMember(const std::string &name)
{
    GuildMember *m;
    if ((m = getMember(name)))
    {
        return m;
    }

    m = new GuildMember(this, name);

    mMembers.push_back(m);

    return m;
}

GuildMember *Guild::getMember(int id) const
{
    for (auto member : mMembers)
        if (member->mId == id)
            return member;

    return nullptr;
}

GuildMember *Guild::getMember(const std::string &name) const
{
    for (auto member : mMembers)
        if (member->getName() == name)
            return member;

    return nullptr;
}

void Guild::removeMember(GuildMember *member)
{
    for (auto itr = mMembers.begin(), itr_end = mMembers.end();
         itr != itr_end; ++itr)
    {
        if ((*itr)->mId == member->mId &&
            (*itr)->getName() == member->getName())
        {
            mMembers.erase(itr);
        }
    }
}

void Guild::removeMember(int id)
{
    for (auto itr = mMembers.begin(), itr_end = mMembers.end();
         itr != itr_end; ++itr)
    {
        if ((*itr)->mId == id)
            mMembers.erase(itr);
    }
}

void Guild::removeMember(const std::string &name)
{
    for (auto itr = mMembers.begin(), itr_end = mMembers.end();
         itr != itr_end; ++itr)
    {
        if ((*itr)->getName() == name)
        {
            mMembers.erase(itr);
        }
    }
}

void Guild::removeFromMembers()
{
    for (auto member : mMembers)
    {
        Being *b = actorSpriteManager->findBeing(member->getID());
        b->removeGuild(getId());
    }
}

Avatar *Guild::getAvatarAt(int index)
{
    return mMembers[index];
}

void Guild::setRights(short rights)
{
    // to invite, rights must be greater than 0
    if (rights > 0)
        mCanInviteUsers = true;
}

bool Guild::isMember(GuildMember *member) const
{
    if (member->mGuild != nullptr && member->mGuild != this)
        return false;

    for (auto mMember : mMembers)
    {
        if (mMember->mId == member->mId &&
            mMember->getName() == member->getName())
        {
            return true;
        }
    }

    return false;
}

bool Guild::isMember(int id) const
{
    return getMember(id) != nullptr;
}

bool Guild::isMember(const std::string &name) const
{
    return getMember(name) != nullptr;
}

void Guild::getNames(std::vector<std::string> &names) const
{
    names.clear();

    for (auto member : mMembers)
        names.push_back(member->getName());
}

Guild *Guild::getGuild(int id)
{
    auto it = guilds.find(id);
    if (it != guilds.end())
        return it->second;

    return new Guild(id);
}
