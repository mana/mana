/*
 *  The Mana Client
 *  Copyright (C) 2010-2012  The Mana Developers
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

#include "party.h"

#include "actorspritemanager.h"

#include "net/net.h"

PartyMember::PartyMember(Party *party, int id, const std::string &name):
        Avatar(name), mId(id), mParty(party), mLeader(false)
{
}

Party::PartyMap Party::parties;

Party::Party(short id):
    mId(id),
    mCanInviteUsers(false)
{
    parties[id] = this;
}

Party::~Party()
{
    clearMembers();
}

PartyMember *Party::addMember(int id, const std::string &name)
{
    PartyMember *m;
    if (Net::getNetworkType() == ServerInfo::TMWATHENA && (m = getMember(id)))
    {
        return m;
    }

    m = new PartyMember(this, id, name);

    mMembers.push_back(m);

    return m;
}

PartyMember *Party::getMember(int id) const
{
    MemberList::const_iterator itr = mMembers.begin(),
                               itr_end = mMembers.end();
    while (itr != itr_end)
    {
        if ((*itr)->mId == id)
        {
            return (*itr);
        }
        ++itr;
    }

    return NULL;
}

PartyMember *Party::getMember(const std::string &name) const
{
    MemberList::const_iterator itr = mMembers.begin(),
                               itr_end = mMembers.end();
    while (itr != itr_end)
    {
        if ((*itr)->getName() == name)
        {
            return (*itr);
        }
        ++itr;
    }

    return NULL;
}

void Party::removeMember(PartyMember *member)
{
    MemberList::iterator itr = mMembers.begin(),
                               itr_end = mMembers.end();
    while(itr != itr_end)
    {
        if((*itr)->mId == member->mId &&
           (*itr)->getName() == member->getName())
        {
            PartyMember *member = (*itr);
            mMembers.erase(itr);
            delete member;
        }
        ++itr;
    }
}

void Party::removeMember(int id)
{
    MemberList::iterator itr = mMembers.begin(),
                               itr_end = mMembers.end();
    while(itr != itr_end)
    {
        if((*itr)->mId == id)
        {
            PartyMember *member = (*itr);
            mMembers.erase(itr);
            delete member;
        }
        ++itr;
    }
}

void Party::removeMember(const std::string &name)
{
    MemberList::iterator itr = mMembers.begin(),
                               itr_end = mMembers.end();
    while(itr != itr_end)
    {
        if((*itr)->getName() == name)
        {
            PartyMember *member = (*itr);
            mMembers.erase(itr);
            delete member;
        }
        ++itr;
    }
}

void Party::removeFromMembers()
{
    MemberList::iterator itr = mMembers.begin(),
                               itr_end = mMembers.end();
    while(itr != itr_end)
    {
        Being *b = actorSpriteManager->findBeing((*itr)->getID());
        if (b)
            b->setParty(NULL);
        ++itr;
    }
}

Avatar *Party::getAvatarAt(int index)
{
    return mMembers[index];
}

void Party::setRights(short rights)
{
    // to invite, rights must be greater than 0
    if (rights > 0)
    {
        mCanInviteUsers = true;
    }
}

bool Party::isMember(PartyMember *member) const
{
    if (member->mParty > 0 && member->mParty != this)
        return false;

    MemberList::const_iterator itr = mMembers.begin(),
                                     itr_end = mMembers.end();
    while (itr != itr_end)
    {
        if ((*itr)->mId == member->mId &&
            (*itr)->getName() == member->getName())
        {
            return true;
        }
        ++itr;
    }

    return false;
}

bool Party::isMember(int id) const
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

bool Party::isMember(const std::string &name) const
{
    MemberList::const_iterator itr = mMembers.begin(),
                                     itr_end = mMembers.end();
    while (itr != itr_end)
    {
        if ((*itr)->getName() == name)
        {
            return true;
        }
        ++itr;
    }

    return false;
}

void Party::getNames(std::vector<std::string> &names) const
{
    names.clear();
    MemberList::const_iterator it = mMembers.begin(),
                                     it_end = mMembers.end();
    while (it != it_end)
    {
        names.push_back((*it)->getName());
        ++it;
    }
}

Party *Party::getParty(int id)
{
    PartyMap::iterator it = parties.find(id);
    if (it != parties.end())
        return it->second;

    return new Party(id);
}
