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
    Avatar(name), mId(id), mParty(party)
{
}

Party::PartyMap Party::parties;

Party::Party(short id):
    mId(id)
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
    for (auto member : mMembers)
        if (member->mId == id)
            return member;

    return nullptr;
}

PartyMember *Party::getMember(const std::string &name) const
{
    for (auto member : mMembers)
        if (member->getName() == name)
            return member;

    return nullptr;
}

void Party::removeMember(PartyMember *member)
{
    for (auto itr = mMembers.begin(), itr_end = mMembers.end();
         itr != itr_end; ++itr)
    {
        if((*itr)->mId == member->mId &&
           (*itr)->getName() == member->getName())
        {
            PartyMember *member = (*itr);
            mMembers.erase(itr);
            delete member;
        }
    }
}

void Party::removeMember(int id)
{
    for (auto itr = mMembers.begin(), itr_end = mMembers.end();
         itr != itr_end; ++itr)
    {
        if ((*itr)->mId == id)
        {
            PartyMember *member = (*itr);
            mMembers.erase(itr);
            delete member;
        }
    }
}

void Party::removeMember(const std::string &name)
{
    for (auto itr = mMembers.begin(), itr_end = mMembers.end();
         itr != itr_end; ++itr)
    {
        if ((*itr)->getName() == name)
        {
            PartyMember *member = (*itr);
            mMembers.erase(itr);
            delete member;
        }
    }
}

void Party::removeFromMembers()
{
    for (auto member : mMembers)
        if (Being *b = actorSpriteManager->findBeing(member->getID()))
            b->setParty(nullptr);
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
    if (member->mParty != nullptr && member->mParty != this)
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

bool Party::isMember(int id) const
{
    for (auto member : mMembers)
        if (member->mId == id)
            return true;

    return false;
}

bool Party::isMember(const std::string &name) const
{
    for (auto member : mMembers)
        if (member->getName() == name)
            return true;

    return false;
}

void Party::getNames(std::vector<std::string> &names) const
{
    names.clear();
    for (auto member : mMembers)
        names.push_back(member->getName());
}

Party *Party::getParty(int id)
{
    auto it = parties.find(id);
    if (it != parties.end())
        return it->second;

    return new Party(id);
}
