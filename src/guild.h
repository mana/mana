/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef GUILD_H
#define GUILD_H

#include "avatar.h"

#include "gui/widgets/avatarlistbox.h"

#include <map>
#include <string>
#include <vector>

class Guild;

class GuildMember : public Avatar
{
public:
    GuildMember(int guildId, int id, const std::string &name);

    GuildMember(int guildId, int id);

    GuildMember(int guildId, const std::string &name);

    int getID() const { return mId; }

    void setID(int id) { mId = id; }

    Guild *getGuild() const { return mGuild; }

protected:
    friend class Guild;

    int mId;
    Guild *mGuild;
};

class Guild : public AvatarListModel
{
public:

    /**
     * Set the guild's name.
     */
    void setName(const std::string &name)
    {
        mName = name;
    }

    /**
     * Adds member to the list.
     */
    void addMember(GuildMember *member);

    /**
     * Find a member by ID.
     *
     * @return the member with the given ID, or NULL if they don't exist.
     */
    GuildMember *getMember(int id);

    /**
     * Find a member by name.
     *
     * @return the member with the given name, or NULL if they don't exist.
     */
    GuildMember *getMember(std::string name);

    /**
     * Get the name of the guild.
     * @return returns name of the guild
     */
    const std::string &getName() const
    {
        return mName;
    }

    /**
     * Get the id of the guild.
     * @return Returns the id of the guild
     */
    short getId() const
    {
        return mId;
    }

    /**
     * Removes a member from the guild.
     */
    void removeMember(GuildMember *member);

    /**
     * Removes a member from the guild.
     */
    void removeMember(int id);

    /**
     * Removes a member from the guild.
     */
    void removeMember(const std::string &name);

    void removeFromMembers();

    void clearMembers() { mMembers.clear(); }

    /**
     * Get size of members list.
     * @return Returns the number of members in the guild.
     */
    int getNumberOfElements() {
        return mMembers.size();
    }

    Avatar *getAvatarAt(int i);

    /**
     * Get whether user can invite users to this guild.
     * @return Returns true if user can invite users
     */
    bool getInviteRights() const
    {
        return mCanInviteUsers;
    }

    void setRights(short rights);

    bool isMember(GuildMember *member) const;

    bool isMember(int id) const;

    bool isMember(const std::string &name) const;

    const void getNames(std::vector<std::string> &names) const;

    static Guild *getGuild(int id);

private:
    typedef std::map<int, Guild*> GuildMap;
    static GuildMap guilds;

    /**
     * Constructor with guild id passed to it.
     */
    Guild(short id);

    typedef std::vector<GuildMember*> MemberList;
    MemberList mMembers;
    std::string mName;
    short mId;
    bool mCanInviteUsers;
};

#endif // GUILD_H
