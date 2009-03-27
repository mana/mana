/*
 *  The Mana World
 *  Copyright (C) 2008  Lloyd Bryant <lloyd_bryant@netzero.net>
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

#ifndef PARTY_H
#define PARTY_H

#include <string>

#include <guichan/actionlistener.hpp>

class PartyHandler;
class Being;
class ChatWindow;

class Party
{
    public:
        Party();
        void respond(const std::string &command, const std::string &args);

        void create(const std::string &party);
        void leave(const std::string &args);

        void createResponse(bool ok);
        void inviteResponse(const std::string &nick, int status);
        void invitedAsk(const std::string &nick, int gender,
                        const std::string &partyName);
        void leftResponse(const std::string &nick);
        void receiveChat(Being *being, const std::string &msg);

        void help(const std::string &args);

    private:
        std::string mPartyName;
        bool mInParty;
        bool mCreating; /**< Used to give an appropriate response to
                          failure */
        PartyHandler *handler;

        class InviteListener : public gcn::ActionListener
        {
            public:
                InviteListener(bool *inParty) :
                    mInParty(inParty)
                {}
                void action(const gcn::ActionEvent &event);

            private:
                bool *mInParty;
        };
        InviteListener mInviteListener;
};

extern Party *playerParty;

#endif
