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
 *
 *  $Id: box.cpp 1456 2005-07-15 23:17:00Z b_lindeijer $
 */

#include "partywindow.h"
#include "chat.h"

#include "../utils/gettext.h"
#include "../net/chatserver/party.h"

PartyWindow::PartyWindow() : Window(_("Party"))
{
    setVisible(false);
}

PartyWindow::~PartyWindow()
{
    PartyList::iterator itr = mPartyMembers.begin(),
                        itr_end = mPartyMembers.end();

    while (itr != itr_end)
    {
        delete (*itr);
    }

    mPartyMembers.clear();
}

void PartyWindow::draw(gcn::Graphics *graphics)
{

}

void PartyWindow::addPartyMember(const std::string &memberName)
{
    PartyMember *player = new PartyMember;
    PartyList::iterator itr = mPartyMembers.begin(),
                        itr_end = mPartyMembers.end();

    while (itr != itr_end)
    {
        if ((*itr)->name == memberName)
        {
            return;
        }
        ++itr;
    }

    player->name = memberName;
    mPartyMembers.push_back(player);

    if (mPartyMembers.size() > 1)
    {
        setVisible(true);
    }
}

void PartyWindow::removePartyMember(const std::string &memberName)
{
    PartyList::iterator itr = mPartyMembers.begin(),
                        itr_end = mPartyMembers.end();

    while (itr != itr_end)
    {
        if ((*itr)->name == memberName)
        {
            mPartyMembers.erase(itr);
            break;
        }
        ++itr;
    }

    if (mPartyMembers.size() < 1)
    {
        setVisible(false);
    }
}

void PartyWindow::showPartyInvite(const std::string &inviter)
{
    if (mPartyInviter != "")
    {
        chatWindow->chatLog("Received party request, but one already exists",
                            BY_SERVER);
        return;
    }
    std::string msg = inviter + " has invited you to join their party";
    chatWindow->chatLog(msg, BY_SERVER);

    acceptDialog = new ConfirmDialog("Accept Party Invite", msg, this);
    acceptDialog->addActionListener(this);

    mPartyInviter = inviter;
}

void PartyWindow::action(const gcn::ActionEvent &event)
{
    const std::string &eventId = event.getId();

    if (eventId == "yes")
    {
        Net::ChatServer::Party::acceptInvite(mPartyInviter);
        mPartyInviter = "";
        delete acceptDialog;
    }
    else if (eventId == "no")
    {
        mPartyInviter = "";
        delete acceptDialog;
    }
}
