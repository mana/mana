#ifndef TMW_GUILD_H
#define TMW_GUILD_H

#include <string>
#include <guichan/listmodel.hpp>

#include "player.h"

class Guild : public gcn::ListModel
{
public:
    /**
     * Constructor with guild id passed to it
     */
    Guild(short id, bool inviteRights);

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
    const std::string& getName() const
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
