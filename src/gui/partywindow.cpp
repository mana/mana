/*
 *  The Mana World
 *  Copyright 2008 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "partywindow.h"
#include "chat.h"

#include "widgets/avatar.h"

#include "../utils/gettext.h"
#include "../net/tmwserv/chatserver/party.h"

PartyWindow::PartyWindow() : Window(_("Party"))
{
    setWindowName("Party");
    setVisible(false);
    setResizable(false);
    setCaption(_("Party"));
    setCloseButton(true);
    setMinWidth(110);
    setMinHeight(200);
    setDefaultSize(620, 300, 110, 200);

    loadWindowState();
}

PartyWindow::~PartyWindow()
{
    mPartyMembers.clear();
}

void PartyWindow::draw(gcn::Graphics *graphics)
{
    Window::draw(graphics);
}

void PartyWindow::addPartyMember(const std::string &memberName)
{
    // check to see if player is already in the party
    PartyList::iterator itr = mPartyMembers.begin(),
                        itr_end = mPartyMembers.end();

    while (itr != itr_end)
    {
        if ((*itr).name == memberName)
        {
            // already in the party, dont add
            return;
        }
        ++itr;
    }

    // create new party member
    PartyMember player;
    player.name = memberName;
    mPartyMembers.push_back(player);

    // add avatar of the new member to window
    Avatar *avatar = new Avatar(memberName);
    add(avatar, 0, (mPartyMembers.size() - 1)*14);

    // show the window
    if (mPartyMembers.size() > 0)
    {
        setVisible(true);
    }
}

void PartyWindow::removePartyMember(const std::string &memberName)
{
    // remove the party member
    PartyList::iterator itr = mPartyMembers.begin(),
                        itr_end = mPartyMembers.end();

    while (itr != itr_end)
    {
        if ((*itr).name == memberName)
        {
            mPartyMembers.erase(itr);
            break;
        }
        ++itr;
    }

    // if no-one left, remove the party window
    if (mPartyMembers.size() < 1)
    {
        setVisible(false);
    }
}

void PartyWindow::showPartyInvite(const std::string &inviter)
{
    // check there isnt already an invite showing
    if (mPartyInviter != "")
    {
        chatWindow->chatLog("Received party request, but one already exists",
                            BY_SERVER);
        return;
    }

    // log invite
    std::string msg = inviter + " has invited you to join their party";
    chatWindow->chatLog(msg, BY_SERVER);

    // show invite
    acceptDialog = new ConfirmDialog("Accept Party Invite", msg, this);
    acceptDialog->addActionListener(this);

    mPartyInviter = inviter;
}

void PartyWindow::action(const gcn::ActionEvent &event)
{
    const std::string &eventId = event.getId();

    // check if they accepted the invite
    if (eventId == "yes")
    {
        chatWindow->chatLog("Accepted invite from " + mPartyInviter);
        Net::ChatServer::Party::acceptInvite(mPartyInviter);
        mPartyInviter = "";
    }
    else if (eventId == "no")
    {
        mPartyInviter = "";
    }
}
