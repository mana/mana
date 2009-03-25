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

#ifndef PARTYWINDOW_H
#define PARTYWINDOW_H

#include "window.h"
#include "confirm_dialog.h"

#include <string>
#include <vector>

#include <guichan/actionevent.hpp>
#include <guichan/actionlistener.hpp>

/**
 * Party Member
 * Used for storing players in the party
 */
struct PartyMember
{
    std::string name;
    int vitality;
};

/**
 * Party Window.
 *
 * \ingroup Interface
 */
class PartyWindow : public Window, gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        PartyWindow();

        /**
         * Release all the players created
         */
        ~PartyWindow();

        /**
         * Draws the party window
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Add party member
         */
        void addPartyMember(const std::string &memberName);

        /**
         * Remove party member
         */
        void removePartyMember(const std::string &memberName);

        /**
         * Show party invite
         */
        void showPartyInvite(const std::string &inviter);

        /**
         * Handle events
         */
        void action(const gcn::ActionEvent &event);

    private:
        typedef std::vector<PartyMember> PartyList;
        PartyList mPartyMembers;
        std::string mPartyInviter;
        ConfirmDialog *acceptDialog;
};

extern PartyWindow *partyWindow;

#endif
