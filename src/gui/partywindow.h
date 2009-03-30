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

#include "gui/widgets/avatar.h"

#include <string>
#include <map>

#include <guichan/actionevent.hpp>
#include <guichan/actionlistener.hpp>

/**
 * Party Member
 * Used for storing players in the party
 */
struct PartyMember
{
    int id;
    std::string name;
    int health;
    int healthMax;
    bool leader;
    bool online;
    Avatar *avatar;
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
         * Find a party member based on ID
         */
        PartyMember *findMember(int id) { return mMembers[id]; }

        /**
         * Find a party member based on ID. Creates if it doesn't already exist.
         */
        PartyMember *findMember2(int id);

        /**
         * Returns the id of the first member found with the given name or -1
         * if it isn't found.
         */
        int findMember(const std::string &name);

        /**
         * Update/add a party member
         */
        void updateMember(int id, const std::string &memberName,
                            bool leader = false, bool online = true);

        /**
         * Remove party member
         */
        void removeMember(int id);

        /**
         * Remove party member
         */
        void removeMember(const std::string &name);

        /**
         * Remove party member
         */
        void updateOnlne(int id, bool online);

        /**
         * Show party invite
         */
        void showPartyInvite(const std::string &inviter,
                             const std::string &partyName = "");

        /**
         * Handle events
         */
        void action(const gcn::ActionEvent &event);

    private:
        typedef std::map<int, PartyMember*> PartyList;
        PartyList mMembers;
        std::string mPartyInviter;
        ConfirmDialog *acceptDialog;
};

extern PartyWindow *partyWindow;

#endif
