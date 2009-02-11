/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
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

#include "being.h"
#include "localplayer.h"
#include "party.h"

#include "gui/chat.h"
#include "gui/confirm_dialog.h"

#include "net/messageout.h"
#include "net/protocol.h"

#include "utils/gettext.h"
#include "utils/strprintf.h"

Party::Party(ChatWindow *chat, Network *network) :
    mChat(chat),
    mNetwork(network),
    mInviteListener(network, &mInParty)
{
}

void Party::respond(const std::string &command, const std::string &args)
{
    if (command == "new" || command == "create")
    {
        create(args);
        return;
    }
    if (command == "leave")
    {
        leave(args);
        return;
    }
    if (command == "settings")
    {
        mChat->chatLog(_("Not yet implemented!"), BY_SERVER);
        return;
        /*
        MessageOut outMsg(mNetwork);
        outMsg.writeInt16(CMSG_PARTY_SETTINGS);
        outMsg.writeInt16(0); // Experience
        outMsg.writeInt16(0); // Item
        */
    }
    mChat->chatLog(_("Party command not known."), BY_SERVER);
}

void Party::create(const std::string &party)
{
    if (party.empty())
    {
        mChat->chatLog(_("Party name is missing."), BY_SERVER);
        return;
    }
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_PARTY_CREATE);
    outMsg.writeString(party.substr(0, 23), 24);
    mCreating = true;
}

void Party::leave(const std::string &args)
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_PARTY_LEAVE);
    mChat->chatLog(_("Left party."), BY_SERVER);
    mInParty = false;
}

void Party::createResponse(bool ok)
{
    if (ok)
    {
        mChat->chatLog(_("Party successfully created."), BY_SERVER);
        mInParty = true;
    }
    else
    {
        mChat->chatLog(_("Could not create party."), BY_SERVER);
    }
}

void Party::inviteResponse(const std::string &nick, int status)
{
    switch (status)
    {
        case 0:
            mChat->chatLog(strprintf(_("%s is already a member of a party."),
                        nick.c_str()), BY_SERVER);
            break;
        case 1:
            mChat->chatLog(strprintf(_("%s refused your invitation."),
                        nick.c_str()), BY_SERVER);
            break;
        case 2:
            mChat->chatLog(strprintf(_("%s is now a member of your party."),
                        nick.c_str()), BY_SERVER);
            break;
    }
}

void Party::invitedAsk(const std::string &nick, int gender,
                       const std::string &partyName)
{
    mPartyName = partyName; /* Quick and nasty - needs redoing */
    if (nick.empty())
    {
        mChat->chatLog(_("You can\'t have a blank party name!"), BY_SERVER);
        return;
    }
    mCreating = false;
    ConfirmDialog *dlg = new ConfirmDialog(_("Invite to party"),
            strprintf(_("%s invites you to join"
                        " the %s party, do you accept?"),
                nick.c_str(), partyName.c_str()));
    dlg->addActionListener(&mInviteListener);
}

void Party::InviteListener::action(const gcn::ActionEvent &event)
{
    MessageOut outMsg(mNetwork);
    outMsg.writeInt16(CMSG_PARTY_INVITED);
    outMsg.writeInt32(player_node->getId());
    bool accept = event.getId() == "yes";
    outMsg.writeInt32(accept ? 1 : 0);
    *mInParty = *mInParty || accept;
}

void Party::leftResponse(const std::string &nick)
{
    mChat->chatLog(strprintf(_("%s has left your party."), nick.c_str()),
                               BY_SERVER);
}

void Party::receiveChat(Being *being, const std::string &msg)
{
    if (!being)
    {
        return;
    }
    if (being->getType() != Being::PLAYER)
    {
        mChat->chatLog(_("Party chat received, but being is not a player"),
                       BY_SERVER);
        return;
    }
    being->setSpeech(msg, SPEECH_TIME);
    mChat->chatLog(being->getName() + " : " + msg, BY_PARTY);
}

void Party::help(const std::string &msg)
{
    if (msg.empty())
    {
        mChat->chatLog(_("Command: /party <command> <args>"), BY_SERVER);
        mChat->chatLog(_("where <command> can be one of:"), BY_SERVER);
        mChat->chatLog(_("   /new"), BY_SERVER);
        mChat->chatLog(_("   /create"), BY_SERVER);
        mChat->chatLog(_("   /prefix"), BY_SERVER);
        mChat->chatLog(_("   /leave"), BY_SERVER);
        mChat->chatLog(_("This command implements the partying function."),
                       BY_SERVER);
        mChat->chatLog(_("Type /help party <command> for further help."),
                       BY_SERVER);
        return;
    }
    if (msg == "new" || msg == "create")
    {
        mChat->chatLog(_("Command: /party new <party-name>"), BY_SERVER);
        mChat->chatLog(_("Command: /party create <party-name>"), BY_SERVER);
        mChat->chatLog(_("These commands create a new party <party-name."),
                BY_SERVER);
        return;
    }
    if (msg == "prefix")
    {
        mChat->chatLog(_("Command: /party prefix <prefix-char>"), BY_SERVER);
        mChat->chatLog(_("This command sets the party prefix character."),
                       BY_SERVER);
        mChat->chatLog(_("Any message preceded by <prefix-char> is sent to "
                         "the party instead of everyone."), BY_SERVER);
        mChat->chatLog(_("Command: /party prefix"), BY_SERVER);
        mChat->chatLog(_("This command reports the current party prefix "
                         "character."), BY_SERVER);
        return;
    }
    //if (msg == "settings")
    //if (msg == "info")
    if (msg == "leave")
    {
        mChat->chatLog(_("Command: /party leave"), BY_SERVER);
        mChat->chatLog(_("This command causes the player to leave the party."),
                BY_SERVER);
        return;
    }
    mChat->chatLog(_("Unknown /party command."), BY_SERVER);
    mChat->chatLog(_("Type /help party for a list of options."), BY_SERVER);
}
