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

#include "../utils/gettext.h"

PartyWindow::PartyWindow() : Window(_("Party"))
{
    setVisible(false);
}

void PartyWindow::draw(gcn::Graphics *graphics)
{

}

void PartyWindow::addPartyMember(Player *player)
{
    PartyList::iterator itr = std::find(mPartyMembers.begin(),
                                        mPartyMembers.end(),
                                        player);
    if (itr == mPartyMembers.end())
    {
        mPartyMembers.push_back(player);
    }

    if (mPartyMembers.size() > 1)
    {
        setVisible(true);
    }
}

void PartyWindow::removePartyMember(Player *player)
{
    PartyList::iterator itr = std::find(mPartyMembers.begin(),
                                        mPartyMembers.end(),
                                        player);
    if (itr != mPartyMembers.end())
    {
        mPartyMembers.erase(itr);
    }

    if (mPartyMembers.size() < 1)
    {
        setVisible(false);
    }
}

void PartyWindow::showPartyInvite(const std::string &inviter)
{

}
