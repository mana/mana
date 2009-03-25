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

#ifndef GUILD_H
#define GUILD_H

#include <guichan/listmodel.hpp>

#include <string>
#include <vector>

class Guild : public gcn::ListModel
{
public:
    /**
     * Constructor with guild id passed to it
     */
    Guild(short id, short rights);

    /**
     * Set the guild's name
     */
    void setName(const std::string &name)
    {
        mName = name;
    }

    /**
     * Add member to the list
     */
    void addMember(const std::string &name);

    /**
     * Get the name of the guild
     * @return returns name of the guild
     */
    const std::string &getName() const
    {
        return mName;
    }

    /**
     * Get the id of the guild
     * @return Returns the id of the guild
     */
    short getId() const
    {
        return mId;
    }

    /**
     * Remove member from the guild
     */
    void removeMember(const std::string &name);

    /**
     * Get size of members list
     * @return Returns the number of members in the guild.
     */
    int getNumberOfElements() {
        return mMembers.size();
    }

    /**
     * Get member at i
     * @return Returns the name of member.
     */
    std::string getElementAt(int i) {
        return mMembers[i];
    }

    /**
     * Get whether user can invite users to this guild
     * @return Returns true if user can invite users
     */
    bool getInviteRights()
    {
        return mCanInviteUsers;
    }

    bool isMember(const std::string &name);

private:
    std::string mName;
    short mId;
    std::vector<std::string> mMembers;
    bool mCanInviteUsers;
};

#endif
