/*
 *  The Mana Client
 *  Copyright (C) 2010  The Mana Developers
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

#ifndef PARTY_H
#define PARTY_H

#include "avatar.h"

#include "gui/widgets/avatarlistbox.h"

#include <map>
#include <string>
#include <vector>

class Party;

class PartyMember : public Avatar
{
public:
    PartyMember(int partyId, int id, const std::string &name);

    PartyMember(int partyId, int id);

    PartyMember(int partyId, const std::string &name);

    int getID() const { return mId; }

    void setID(int id) { mId = id; }

    Party *getParty() const { return mParty; }

    bool getLeader() const { return mLeader; }

    void setLeader(bool leader) { mLeader = leader; setDisplayBold(leader); }

protected:
    friend class Party;

    int mId;
    Party *mParty;
    bool mLeader;
};

class Party : public AvatarListModel
{
public:

    /**
     * Set the party's name.
     */
    void setName(const std::string &name)
    {
        mName = name;
    }

    /**
     * Adds member to the list.
     */
    void addMember(PartyMember *member);

    /**
     * Find a member by ID.
     *
     * @return the member with the given ID, or NULL if they don't exist.
     */
    PartyMember *getMember(int id);

    /**
     * Find a member by name.
     *
     * @return the member with the given name, or NULL if they don't exist.
     */
    PartyMember *getMember(std::string name);

    /**
     * Get the name of the party.
     * @return returns name of the party
     */
    const std::string &getName() const
    {
        return mName;
    }

    /**
     * Get the id of the party.
     * @return Returns the id of the party
     */
    short getId() const
    {
        return mId;
    }

    /**
     * Removes a member from the party.
     */
    void removeMember(PartyMember *member);

    /**
     * Removes a member from the party.
     */
    void removeMember(int id);

    /**
     * Removes a member from the party.
     */
    void removeMember(const std::string &name);

    void clearMembers() { mMembers.clear(); }

    void removeFromMembers();

    /**
     * Get size of members list.
     * @return Returns the number of members in the party.
     */
    int getNumberOfElements() {
        return mMembers.size();
    }

    Avatar *getAvatarAt(int i);

    /**
     * Get whether user can invite users to this party.
     * @return Returns true if user can invite users
     */
    bool getInviteRights() const
    {
        return mCanInviteUsers;
    }

    void setRights(short rights);

    bool isMember(PartyMember *member) const;

    bool isMember(int id) const;

    bool isMember(const std::string &name) const;

    void getNames(std::vector<std::string> &names) const;

    static Party *getParty(int id);

private:
    typedef std::map<int, Party*> PartyMap;
    static PartyMap parties;

    /**
     * Constructor with party id passed to it.
     */
    Party(short id);

    typedef std::vector<PartyMember*> MemberList;
    MemberList mMembers;
    std::string mName;
    short mId;
    bool mCanInviteUsers;
};

#endif // PARTY_H
