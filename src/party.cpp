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
 *
 *  $Id: party.cpp
 */

#include "party.h"

#include "beingmanager.h"
#include "localplayer.h"
#include "game.h"

#include "gui/chat.h"
#include "gui/confirm_dialog.h"

#include "net/messageout.h"
#include "net/protocol.h"

Party::Party(ChatWindow *chat, Network *network) :
             mChat(chat), mNetwork(network), mInviteListener(network, &mInParty)
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
	mChat->chatLog("Not yet implemented!", BY_SERVER);
	return;
	/*
	  MessageOut outMsg(mNetwork);
	  outMsg.writeInt16(CMSG_PARTY_SETTINGS);
	  outMsg.writeInt16(0); // Experience
	  outMsg.writeInt16(0); // Item
	 */
    }
    mChat->chatLog("Party command not known.", BY_SERVER);
}

void Party::create(const std::string &party)
{
    if (party == "")
    {
	mChat->chatLog("Party name is missing.", BY_SERVER);
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
    mChat->chatLog("Left party.", BY_SERVER);
    mInParty = false;
}

void Party::createResponse(bool ok)
{
    if (ok)
    {
	mChat->chatLog("Party successfully created.", BY_SERVER);
	mInParty = true;
    }
    else
    {
	mChat->chatLog("Could not create party.", BY_SERVER);
    }
}

void Party::inviteResponse(const std::string &nick, int status)
{
    switch (status)
    {
        case 0:
  	    mChat->chatLog(nick + " is already a member of a party.",
			   BY_SERVER);
	    break;
        case 1:
  	    mChat->chatLog(nick + " refused your invitation.", BY_SERVER);
	    break;
        case 2:
 	    mChat->chatLog(nick + " is now a member of your party.",
			   BY_SERVER);
	    break;
    }
}

void Party::invitedAsk(const std::string &nick, int gender,
		       const std::string &partyName)
{
    mPartyName = partyName; /* Quick and nasty - needs redoing */
    if (nick == "")
    {
        mChat->chatLog("Something\'s wrong!", BY_SERVER);
	return;
    }
    mCreating = false;
    ConfirmDialog *dlg = new ConfirmDialog("Invite to party",
					   nick + " invites you to join " +
					   (gender == 0 ? "his" : "her") +
					   " party, " + partyName +
					   ", do you accept?");
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
    mChat->chatLog(nick + " has left your party.", BY_SERVER);
}

void Party::receiveChat(Being *being, const std::string &msg)
{
    if (being == NULL)
    {
	return;
    }
    if (being->getType() != Being::PLAYER)
    {
	mChat->chatLog("Something\'s wrong!", BY_SERVER);
	return;
    }
    being->setSpeech(msg, SPEECH_TIME);
    mChat->chatLog(being->getName() + " : " + msg, BY_PARTY);
}

void Party::help()
{
    mChat->chatLog("/party <command> <params>: Party commands.", BY_SERVER);
}

void Party::help(const std::string &msg)
{
    if (msg == "")
    {
	mChat->chatLog("Command: /party <command> <args>", BY_SERVER);
	mChat->chatLog("where <command> can be one of:", BY_SERVER);
	mChat->chatLog("   /new", BY_SERVER);
	mChat->chatLog("   /create", BY_SERVER);
	mChat->chatLog("   /prefix", BY_SERVER);
	mChat->chatLog("   /leave", BY_SERVER);
	mChat->chatLog("This command implements the partying function.",
		       BY_SERVER);
	mChat->chatLog("Type /help party <command> for further help.",
		       BY_SERVER);
	return;
    }
    if (msg == "new" || msg == "create")
    {
	mChat->chatLog("Command: /party new <party-name>", BY_SERVER);
	mChat->chatLog("Command: /party create <party-name>", BY_SERVER);
	mChat->chatLog("These commands create a new party <party-name.",
		       BY_SERVER);
	return;
    }
    if (msg == "prefix")
    {
	mChat->chatLog("Command: /party prefix <prefix-char>", BY_SERVER);
	mChat->chatLog("This command sets the party prefix character.",
		       BY_SERVER);
	mChat->chatLog("Any message preceded by <prefix-char> is sent to "
		       "the party instead of everyone.", BY_SERVER);
	mChat->chatLog("Command: /party prefix", BY_SERVER);
	mChat->chatLog("This command reports the current party prefix "
		       "character.", BY_SERVER);
	return;
    }
    //if (msg == "settings")
    //if (msg == "info")
    if (msg == "leave")
    {
	mChat->chatLog("Command: /party leave", BY_SERVER);
	mChat->chatLog("This command causes the player to leave the party.",
		       BY_SERVER);
	return;
    }
    mChat->chatLog("Unknown /party command.", BY_SERVER);
    mChat->chatLog("Type /help party for a list of options.", BY_SERVER);
}
