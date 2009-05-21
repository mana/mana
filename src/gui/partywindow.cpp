/*
 *  The Mana World
 *  Copyright (C) 2008  The Mana World Development Team
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

#include "gui/partywindow.h"

#include "gui/widgets/chattab.h"

#include "beingmanager.h"
#include "player.h"

#include "net/net.h"
#include "net/partyhandler.h"

#include "utils/dtor.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"

PartyMember::PartyMember():
    avatar(new Avatar)
{
}

PartyMember::~PartyMember()
{
    delete avatar;
}


PartyWindow::PartyWindow() :
    Window(_("Party"))
{
    setWindowName("Party");
    setVisible(false);
    setSaveVisible(true);
    setResizable(true);
    setSaveVisible(true);
    setCloseButton(true);
    setMinWidth(120);
    setMinHeight(55);
    setDefaultSize(590, 200, 150, 60);

    loadWindowState();
}

PartyWindow::~PartyWindow()
{
    delete_all(mMembers);
}

void PartyWindow::setPartyName(const std::string &name)
{
    setCaption(strprintf(_("Party (%s)"), name.c_str()));
}

void PartyWindow::clearPartyName()
{
    setCaption(_("Party"));
}

PartyMember *PartyWindow::findMember(int id) const
{
    PartyList::const_iterator it = mMembers.find(id);
    if (it == mMembers.end())
        return NULL;
    else
        return it->second;
}

PartyMember *PartyWindow::findOrCreateMember(int id)
{
    PartyMember *member = findMember(id);

    if (!member)
    {
        member = new PartyMember;
        mMembers[id] = member;
    }

    buildLayout();

    return member;
}

int PartyWindow::findMember(const std::string &name) const
{
    PartyList::const_iterator itr = mMembers.begin(),
                              itr_end = mMembers.end();

    while (itr != itr_end)
    {
        if ((*itr).second->name == name)
        {
            return (*itr).first;
        }
        ++itr;
    }

    return -1;
}

void PartyWindow::updateMember(int id, const std::string &memberName,
                               bool leader, bool online)
{
    PartyMember *member = findOrCreateMember(id);
    member->name = memberName;
    member->leader = leader;
    member->online = online;
    member->avatar->setDisplayBold(leader);
    member->avatar->setName(memberName);
    member->avatar->setOnline(online);

    Player *player = dynamic_cast<Player*>(beingManager->findBeing(id));
    if (player && online)
        player->setInParty(true);
}

void PartyWindow::updateMemberHP(int id, int hp, int maxhp)
{
    PartyMember *player = findOrCreateMember(id);
    player->avatar->setHp(hp);
    player->avatar->setMaxHp(maxhp);
}

void PartyWindow::removeMember(int id)
{
    mMembers.erase(id);

    if (Player *player = dynamic_cast<Player*>(beingManager->findBeing(id)))
        player->setInParty(false);
}

void PartyWindow::removeMember(const std::string &name)
{
    removeMember(findMember(name));

    buildLayout();
}

void PartyWindow::updateOnlne(int id, bool online)
{
    PartyMember *player = findMember(id);

    if (!player)
        return;

    player->online = online;
    player->avatar->setOnline(online);
}

void PartyWindow::showPartyInvite(const std::string &inviter,
                                  const std::string &partyName)
{
    // check there isnt already an invite showing
    if (mPartyInviter != "")
    {
        localChatTab->chatLog(_("Received party request, but one already "
                "exists."), BY_SERVER);
        return;
    }

    std::string msg;
    // log invite
    if (partyName.empty())
        msg = strprintf(N_("%s has invited you to join their party."),
                                    inviter.c_str());
    else
        msg = strprintf(N_("%s has invited you to join the %s party."),
                                    inviter.c_str(), partyName.c_str());

    localChatTab->chatLog(msg, BY_SERVER);

    // show invite
    acceptDialog = new ConfirmDialog(_("Accept Party Invite"), msg, this);
    acceptDialog->addActionListener(this);

    mPartyInviter = inviter;
}

void PartyWindow::action(const gcn::ActionEvent &event)
{
    const std::string &eventId = event.getId();

    // check if they accepted the invite
    if (eventId == "yes")
    {
        localChatTab->chatLog(strprintf(_("Accepted invite from %s."),
                mPartyInviter.c_str()));
        Net::getPartyHandler()->inviteResponse(mPartyInviter, true);
        mPartyInviter = "";
    }
    else if (eventId == "no")
    {
        localChatTab->chatLog(strprintf(_("Rejected invite from %s."),
                mPartyInviter.c_str()));
        Net::getPartyHandler()->inviteResponse(mPartyInviter, false);
        mPartyInviter = "";
    }
}

void clearMembersSub(const std::pair<int, PartyMember*> &p)
{
    Player *player = dynamic_cast<Player*>(beingManager->findBeing(p.first));
    if (player)
        player->setInParty(false);
}

void PartyWindow::clearMembers()
{
    clearLayout();

    std::for_each(mMembers.begin(), mMembers.end(), clearMembersSub);

    delete_all(mMembers);
    mMembers.clear();
}

void PartyWindow::buildLayout()
{
    clearLayout();
    int lastPos = 0;

    PartyList::iterator it;
    PartyMember *member;

    for (it = mMembers.begin(); it != mMembers.end(); it++)
    {
        member = (*it).second;
        add(member->avatar, 0, lastPos);
        lastPos += member->avatar->getHeight() + 2;
    }
}
